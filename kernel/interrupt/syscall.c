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
typedef struct { int x; int y; int buttons; } MOUSE_STATE;
extern MOUSE_STATE* MouseGetState(void);

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
     * SEGURIDAD: en v0.3 el proceso GUI corre en Ring 0 con el mismo
     * page directory del kernel, por lo que EDX es una dirección válida.
     * En v0.4 (Ring 3 real) habrá que validar que EDX apunte al espacio
     * de usuario antes de desreferenciar. */
    case SYS_DRAW_STRING: {
        int x        = (int)ctx_ebx(ctx);
        int y        = (int)ctx_ecx(ctx);
        const char*s = (const char*)ctx_edx(ctx);
        uint8_t fg   = (uint8_t)ctx_esi(ctx);
        uint8_t bg   = (uint8_t)ctx_edi(ctx);
        if (s) {
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
    /* Retorna puntero al MOUSE_STATE del kernel.
     * El proceso puede leerlo porque ambos usan el mismo page directory. */
    case SYS_GET_MOUSE: {
        ret = (uint32_t)MouseGetState();
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
