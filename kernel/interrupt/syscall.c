/*
 * syscall.c — Implementación del handler + dispatcher de syscalls
 *
 * El handler INT 0x30 funciona igual que el IRQ0, pero:
 *   - No hace context switch (retorna al mismo proceso)
 *   - El IDT gate tiene DPL=3 para que Ring 3 pueda dispararlo
 *   - Escribe el resultado en EAX del contexto guardado
 *
 * Cada syscall tiene acceso a los servicios del kernel:
 *   - VGA driver: VgaPutPixel, VgaFillRect, VgaDrawString
 *   - Scheduler: scheduler_yield
 *   - Process: proc_exit, proc_current_thread
 *   - Mouse: MouseGetState
 */

#include "syscall.h"
#include "proc/process.h"
#include "proc/scheduler.h"
#include <types.h>

/* ── Servicios de drivers que el kernel expone ──────────────────────── */
/* Forward declarations — los drivers están en drivers/video/vga/ */
extern void  VgaPutPixel(int x, int y, unsigned char color);
extern void  VgaFillRect(int x, int y, int w, int h, unsigned char color);
extern void  VgaDrawString(int x, int y, const char* str,
                            unsigned char fg, unsigned char bg);

/* Mouse */
typedef struct { int x; int y; int buttons; int visible; } MOUSE_STATE;
extern MOUSE_STATE* MouseGetState(void);

/* ── Validación de punteros de usuario ──────────────────────────────────
 *
 * En v0.4, el proceso GUI corre en Ring 3 pero su código y datos estáticos
 * viven físicamente en el espacio del kernel (compilado junto con él).
 * Su page directory mapea 0x00000000..0x07FFFFFF con PTE_USER, por lo que
 * cualquier puntero en ese rango es accesible y válido desde Ring 3.
 *
 * REGLA DE VALIDACION:
 *   - Un puntero es válido si cae en cualquiera de estos rangos:
 *     a) 0x00001000..0x07FFFFFF  (kernel/GUI mapeado con PTE_USER)
 *     b) USER_STACK_BOT..USER_STACK_TOP (stack de usuario Ring 3)
 *   - Rechazamos NULL y la página 0 (null guard).
 *   - Rechazamos punteros al kernel sin mapeo de usuario (>= 0x08000000
 *     y < 0x7FF00000), que serían un intento de acceder a estructuras
 *     del kernel no mapeadas como PTE_USER.
 *
 * NOTA: Esta validación es suficiente para el proyecto. En producción
 * se recorrerían las page tables para verificar que PTE_USER está activo.
 */

/* Rango a): kernel/GUI mapeado con PTE_USER */
#define KERNEL_USER_MAP_END  0x08000000u   /* fin del rango identity-mapped */
/* Rango b): stack de usuario */
#define USER_STACK_BOT       0x7FEF0000u   /* USER_STACK_TOP - USER_STACK_SIZE */
#define USER_STACK_TOP_ADDR  0x7FFF0000u
/* Zona prohibida: entre el mapa del kernel y el stack de usuario */
#define FORBIDDEN_LOW        0x08000000u
#define FORBIDDEN_HIGH       0x7FEF0000u

/* puntero de escritura: solo en stack de usuario (no en .rodata del kernel) */
static inline int user_write_ptr_valid(const void* ptr, uint32_t size)
{
    uint32_t addr = (uint32_t)ptr;
    if (!ptr)                               return 0;   /* NULL */
    if (addr < USER_STACK_BOT)              return 0;   /* fuera del stack */
    if (addr >= USER_STACK_TOP_ADDR)        return 0;   /* pasado el tope */
    if (addr + size < addr)                 return 0;   /* overflow */
    if (addr + size > USER_STACK_TOP_ADDR)  return 0;   /* desborda */
    return 1;
}

/* puntero de lectura: permite tanto el rango del kernel (PTE_USER) como el stack */
static inline int user_read_ptr_valid(const void* ptr, uint32_t size)
{
    uint32_t addr = (uint32_t)ptr;
    if (!ptr) return 0;   /* NULL */
    if (addr < 0x00001000u) return 0;   /* null guard (primera página) */
    if (addr + size < addr) return 0;   /* overflow */
    /* Rango a): 0x00001000..0x07FFFFFF mapeado con PTE_USER */
    if (addr >= 0x00001000u && (addr + size) <= KERNEL_USER_MAP_END)
        return 1;
    /* Rango b): stack de usuario */
    if (addr >= USER_STACK_BOT && (addr + size) <= USER_STACK_TOP_ADDR)
        return 1;
    return 0;   /* zona no mapeada o kernel puro */
}

/* Longitud máxima de string permitida desde usuario */
#define USER_STR_MAX  1024

static int user_str_valid(const char* s)
{
    if (!user_read_ptr_valid(s, 1)) return 0;
    /* Verificar que el string termina dentro del rango permitido */
    const char* p = s;
    int n = 0;
    while (n < USER_STR_MAX) {
        /* Verificar que el byte actual es accesible */
        if (!user_read_ptr_valid(p, 1)) return 0;
        if (*p == '\0') return 1;
        p++;
        n++;
    }
    return 0;   /* string demasiado largo */
}

/* Alias legacy para SYS_GET_MOUSE_STATE (escribe en stack de usuario) */
#define user_ptr_valid(ptr, size) user_write_ptr_valid(ptr, size)

/* Tick counter global (incrementado en cada IRQ0) */
extern volatile uint32_t g_kernel_ticks;

/* ── Handler en assembly ─────────────────────────────────────────────── */
/*
 * INT 0x30 handler — mismo frame que IRQ0 pero sin context switch.
 *
 * Ring 3 → CPU empuja automáticamente: ss_user, esp_user, eflags, cs, eip
 * Nosotros empujamos: err_code ficticio, int_no, pusha, segmentos
 *
 * DIFERENCIA con IRQ0: el gate tiene DPL=3 (0xEE en lugar de 0x8E)
 * para que Ring 3 pueda disparar INT 0x30 sin GPF.
 */
void syscall_handler(void);
extern void syscall_dispatch(cpu_context_t* ctx);

__asm__(
    ".global syscall_handler\n"
    "syscall_handler:\n"

    /* Ficticios para completar el frame cpu_context_t */
    "  pushl $0\n"        /* err_code */
    "  pushl $0x30\n"     /* int_no = 0x30 */

    /* Guardar registros */
    "  pusha\n"
    "  pushl %gs\n"
    "  pushl %fs\n"
    "  pushl %es\n"
    "  pushl %ds\n"

    /* Apuntar segmentos al kernel */
    "  movw $0x10, %ax\n"
    "  movw %ax,   %ds\n"
    "  movw %ax,   %es\n"
    "  movw %ax,   %fs\n"
    "  movw %ax,   %gs\n"

    /* Llamar al dispatcher con el puntero al frame */
    "  movl %esp, %ebx\n"
    "  pushl %ebx\n"
    "  call syscall_dispatch\n"
    "  addl $4, %esp\n"

    /* Restaurar (el dispatcher modificó ctx->eax para el retorno) */
    "  popl %ds\n"
    "  popl %es\n"
    "  popl %fs\n"
    "  popl %gs\n"
    "  popa\n"
    "  addl $8, %esp\n"   /* descartar int_no + err_code */
    "  iret\n"             /* Ring 3: iret restaura también esp_user y ss_user */
);

/* ── Tabla de syscalls ────────────────────────────────────────────────── */

/* Acceso al EAX del contexto guardado para el retorno */
/*
 * cpu_context_t layout (desde el tope del stack, offset 0 = ds):
 *   0:ds 4:es 8:fs 12:gs  16:edi 20:esi 24:ebp 28:esp_d
 *   32:ebx 36:edx 40:ecx 44:eax  48:int_no 52:err_code
 *   56:eip 60:cs 64:eflags 68:user_esp 72:user_ss
 */
static inline uint32_t ctx_eax(cpu_context_t* c) { return c->eax; }
static inline uint32_t ctx_ebx(cpu_context_t* c) { return c->ebx; }
static inline uint32_t ctx_ecx(cpu_context_t* c) { return c->ecx; }
static inline uint32_t ctx_edx(cpu_context_t* c) { return c->edx; }
static inline uint32_t ctx_esi(cpu_context_t* c) { return c->esi; }
static inline uint32_t ctx_edi(cpu_context_t* c) { return c->edi; }

/* ── syscall_dispatch ────────────────────────────────────────────────── */
void syscall_dispatch(cpu_context_t* ctx)
{
    uint32_t num  = ctx_eax(ctx);
    uint32_t ret  = SYSCALL_ERR;

    switch (num) {

    /* ── SYS_EXIT (0x00) ────────────────────────────────────────────── */
    case SYS_EXIT: {
        uint32_t code = ctx_ebx(ctx);
        (void)code;
        proc_exit(code);
        /* proc_exit no retorna */
        ret = SYSCALL_OK;
        break;
    }

    /* ── SYS_YIELD (0x01) ───────────────────────────────────────────── */
    case SYS_YIELD: {
        scheduler_yield();
        ret = SYSCALL_OK;
        break;
    }

    /* ── SYS_DRAW_PIXEL (0x02) ──────────────────────────────────────── */
    /* args: EBX=x, ECX=y, EDX=color */
    case SYS_DRAW_PIXEL: {
        int x     = (int)ctx_ebx(ctx);
        int y     = (int)ctx_ecx(ctx);
        uint8_t c = (uint8_t)ctx_edx(ctx);
        if (x >= 0 && x < 640 && y >= 0 && y < 480) {
            VgaPutPixel(x, y, c);
            ret = SYSCALL_OK;
        }
        break;
    }

    /* ── SYS_FILL_RECT (0x03) ───────────────────────────────────────── */
    /* args: EBX=x, ECX=y, EDX=w, ESI=h, EDI=color */
    case SYS_FILL_RECT: {
        int x     = (int)ctx_ebx(ctx);
        int y     = (int)ctx_ecx(ctx);
        int w     = (int)ctx_edx(ctx);
        int h     = (int)ctx_esi(ctx);
        uint8_t c = (uint8_t)ctx_edi(ctx);
        VgaFillRect(x, y, w, h, c);
        ret = SYSCALL_OK;
        break;
    }

    /* ── SYS_DRAW_STRING (0x04) ─────────────────────────────────────── */
    /* args: EBX=x, ECX=y, EDX=ptr_str (addr virtual usuario), ESI=fg, EDI=bg
     *
     * SEGURIDAD Ring 3: validamos que el puntero EDX apunte al espacio
     * de usuario antes de desreferenciar. Si no es válido, retornamos
     * SYSCALL_ERR sin tocar el hardware. */
    case SYS_DRAW_STRING: {
        int x        = (int)ctx_ebx(ctx);
        int y        = (int)ctx_ecx(ctx);
        const char*s = (const char*)ctx_edx(ctx);
        uint8_t fg   = (uint8_t)ctx_esi(ctx);
        uint8_t bg   = (uint8_t)ctx_edi(ctx);
        /* Validar puntero antes de desreferenciar desde Ring 0 */
        if (user_str_valid(s)) {
            VgaDrawString(x, y, s, fg, bg);
            ret = SYSCALL_OK;
        }
        break;
    }

    /* ── SYS_GET_TICK (0x05) ────────────────────────────────────────── */
    case SYS_GET_TICK: {
        ret = g_kernel_ticks;
        break;
    }

    /* ── SYS_GET_MOUSE (0x06) ───────────────────────────────────────── */
    /* DEPRECATED: retorna puntero directo al kernel — solo seguro desde Ring 0.
     * Mantenido por compatibilidad pero no debe usarse desde Ring 3. */
    case SYS_GET_MOUSE: {
        ret = (uint32_t)MouseGetState();
        break;
    }

    /* ── SYS_GET_MOUSE_STATE (0x07) ────────────────────────────────────── */
    /* Copia el MOUSE_STATE por valor al buffer de usuario apuntado por EBX.
     * SEGURO desde Ring 3: validamos el puntero destino antes de escribir.
     * args: EBX = ptr destino (en espacio de usuario, sizeof MOUSE_STATE bytes)
     * Retorna SYSCALL_OK si se copió, SYSCALL_ERR si el puntero no es válido. */
    case SYS_GET_MOUSE_STATE: {
        MOUSE_STATE* dst = (MOUSE_STATE*)ctx_ebx(ctx);
        if (!user_ptr_valid(dst, sizeof(MOUSE_STATE))) {
            ret = SYSCALL_ERR;
            break;
        }
        MOUSE_STATE* src = MouseGetState();
        /* Copiar campo a campo (no tenemos memcpy en el kernel por ahora) */
        dst->x       = src->x;
        dst->y       = src->y;
        dst->buttons = src->buttons;
        dst->visible = src->visible;
        ret = SYSCALL_OK;
        break;
    }

    default:
        ret = SYSCALL_ERR;
        break;
    }

    /* Escribir el valor de retorno en EAX del contexto guardado.
     * Cuando el handler haga IRET, EAX del proceso quedará con este valor. */
    ctx->eax = ret;
}

/* ── syscall_init ────────────────────────────────────────────────────── */
/*
 * Registra el handler INT 0x30 en la IDT con DPL=3.
 *
 * flags = 0xEE:
 *   bit7   = 1 (Present)
 *   bit6-5 = 11 (DPL=3 — Ring 3 puede disparar este vector)
 *   bit4   = 0 (tipo sistema)
 *   bit3-0 = 1110 (32-bit interrupt gate)
 */
extern void idt_set_gate_pub(uint8_t num, uint32_t base,
                              uint16_t sel, uint8_t flags);

void syscall_init(void)
{
    idt_set_gate_pub(0x30, (uint32_t)syscall_handler, 0x08, 0xEE);
}
