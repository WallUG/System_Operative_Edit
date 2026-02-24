/*
 * IDT + PIC 8259 - Interrupt Descriptor Table & Programmable Interrupt Controller
 *
 * PROBLEMA DIAGNOSTICADO EN QEMU.LOG:
 * El PIC 8259 por defecto mapea IRQ0 (timer) a INT 0x08 e IRQ1 (teclado) a
 * INT 0x09. En modo protegido, 0x08 = Double Fault y 0x09 = Coprocessor
 * Segment Overrun (excepciones de CPU). Cuando llega el primer tick del timer,
 * la CPU lo interpreta como Double Fault → entra al handler de excepcion →
 * genera otro fault → Triple Fault → reset. Eso es exactamente lo que muestra
 * el log: rafagas de INT=0x08 seguidas de INT=0x0e (Page Fault) y muerte.
 *
 * SOLUCION: Remapear el PIC para que IRQ0-7 → INT 0x20-0x27
 *                                       IRQ8-15 → INT 0x28-0x2F
 * y enviar EOI (End Of Interrupt) al final de cada handler.
 */

#include "types.h"
#include "../drivers/video/vga/vga.h"    /* colores VGA para pf_report */
#include "../proc/process.h"   /* cpu_context_t se encuentra en kernel/proc */

/* I/O helpers para puerto serial/com1 0x3F8 */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0,%1"::"a"(val),"Nd"(port));
}
static inline uint8_t inb(uint16_t port) {
    uint8_t v;
    __asm__ volatile("inb %1,%0":"=a"(v):"Nd"(port));
    return v;
}

/* enviar un caracter por COM1 (0x3F8) */
static void serial_putc(char c)
{
    /* esperar THR vacío */
    while (!(inb(0x3F8+5) & 0x20)) ;
    outb(0x3F8, c);
}

/* enviar string por serial */
void serial_puts(const char *s)
{
    while (*s) {
        serial_putc(*s++);
    }
}

/* Inicializa COM1: 115200 baud, 8N1
 * FAQ: esto debe llamarse antes de usar serial_puts. */
void serial_init(void)
{
    outb(0x3F8 + 1, 0x00);    /* Disable all interrupts */
    outb(0x3F8 + 3, 0x80);    /* Enable DLAB (set baud rate divisor) */
    outb(0x3F8 + 0, 0x01);    /* Divisor low byte (115200) */
    outb(0x3F8 + 1, 0x00);    /* Divisor high byte */
    outb(0x3F8 + 3, 0x03);    /* 8 bits, no parity, one stop bit */
    outb(0x3F8 + 2, 0xC7);    /* Enable FIFO, clear them, with 14-byte threshold */
    outb(0x3F8 + 4, 0x0B);    /* IRQs enabled, RTS/DSR set */
}


/* ── Puertos del PIC 8259 ── */
#define PIC1_CMD    0x20
#define PIC1_DATA   0x21
#define PIC2_CMD    0xA0
#define PIC2_DATA   0xA1
#define PIC_EOI     0x20    /* End Of Interrupt */

/* ── Offsets de IRQ tras el remap ── */
#define IRQ_BASE_MASTER 0x20   /* IRQ0-7  → INT 0x20-0x27 */
#define IRQ_BASE_SLAVE  0x28   /* IRQ8-15 → INT 0x28-0x2F */

/* ── Estructura de entrada IDT ── */
struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  zero;
    uint8_t  type_attr;
    uint16_t offset_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

static struct idt_entry idt[256];
static struct idt_ptr   idtp;

/* ── I/O helpers ── */
static inline void outb_idt(uint16_t port, uint8_t val)
{
    __asm__ volatile("outb %0,%1" :: "a"(val), "Nd"(port));
}
static inline uint8_t inb_idt(uint16_t port)
{
    uint8_t v;
    __asm__ volatile("inb %1,%0" : "=a"(v) : "Nd"(port));
    return v;
}
/* Pequeño delay de I/O — necesario entre comandos al PIC */
static inline void io_wait(void)
{
    outb_idt(0x80, 0);   /* Puerto 0x80 = dummy port, solo consume ciclos */
}

/* ═══════════════════════════════════════════════════════
 * PIC 8259 - Remap + Inicialización
 * ═══════════════════════════════════════════════════════ */
static void pic_remap(void)
{
    uint8_t mask1, mask2;

    /* Guardar mascaras actuales */
    mask1 = inb_idt(PIC1_DATA);
    mask2 = inb_idt(PIC2_DATA);

    /* ICW1: iniciar secuencia de inicializacion, modo cascada */
    outb_idt(PIC1_CMD,  0x11); io_wait();
    outb_idt(PIC2_CMD,  0x11); io_wait();

    /* ICW2: nuevos vectores base */
    outb_idt(PIC1_DATA, IRQ_BASE_MASTER); io_wait();   /* Master → 0x20 */
    outb_idt(PIC2_DATA, IRQ_BASE_SLAVE);  io_wait();   /* Slave  → 0x28 */

    /* ICW3: cascada — Master en IRQ2, Slave es esclavo del IRQ2 */
    outb_idt(PIC1_DATA, 0x04); io_wait();   /* Master: IRQ2 tiene esclavo */
    outb_idt(PIC2_DATA, 0x02); io_wait();   /* Slave:  identidad = 2      */

    /* ICW4: modo 8086 */
    outb_idt(PIC1_DATA, 0x01); io_wait();
    outb_idt(PIC2_DATA, 0x01); io_wait();

    /* Restaurar mascaras (o usar 0xFF para silenciar todo excepto lo que
     * habilitemos explicitamente). Silenciamos todo por ahora. */
    outb_idt(PIC1_DATA, 0xFF);
    outb_idt(PIC2_DATA, 0xFF);

    (void)mask1; (void)mask2;
}

/* Enviar EOI al PIC master (y slave si IRQ >= 8) */
static inline void pic_eoi(uint8_t irq)
{
    if (irq >= 8)
        outb_idt(PIC2_CMD, PIC_EOI);
    outb_idt(PIC1_CMD, PIC_EOI);
}

/* ═══════════════════════════════════════════════════════
 * Handlers de excepciones de CPU (0x00-0x1F)
 * Cada uno escribe un codigo de 2 letras en rojo en la
 * esquina superior derecha del buffer de texto VGA y para.
 * ═══════════════════════════════════════════════════════ */

/* Reportar direcciones de page fault */
/* pf_report is referenced from inline assembly in exc_page_fault, so
   cannot be static (otherwise linker drops it). */
/* última dirección que provocó un PF; visible incluso antes de serial */
volatile uint32_t g_last_pf_addr = 0;

void pf_report(uint32_t addr)
{
    /* registrar la dirección para que el código de inicio pueda leerla */
    g_last_pf_addr = addr;

    /* intentar mostrar también en el video-texto por si la serial falla */
    screen_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
    screen_write("PF @ 0x");
    {
        char buf[9];
        for (int i = 0; i < 8; i++)
            buf[i] = "0123456789ABCDEF"[(addr >> ((7 - i) * 4)) & 0xF];
        buf[8] = '\0';
        screen_writeln(buf);
    }

    /* formatear hex para serial */
    char hexstr[9];
    static const char hex[] = "0123456789ABCDEF";
    for (int i = 0; i < 8; i++) {
        hexstr[i] = hex[(addr >> ((7 - i) * 4)) & 0xF];
    }
    hexstr[8] = '\0';

    /* reportar por serial, seguro incluso si modo gráfico está activo */
    serial_puts("PF @ ");
    serial_puts(hexstr);
    serial_puts("\r\n");

    /* cortar CPU para que quede congelado en el punto del fallo */
}

/* reporte de GPF con direcciones para depuración */
void gpf_report(uint32_t eip, uint32_t error)
{
    serial_puts("GPF detected\r\n");
    /* mostrar info adicional */
    char buf[64];
    /* formatear eip y error en hex */
    const char* hex = "0123456789ABCDEF";
    buf[0] = 'E'; buf[1] = 'I'; buf[2] = 'P'; buf[3] = '=';
    for (int i = 0; i < 8; i++) {
        buf[4+i] = hex[(eip >> ((7-i)*4)) & 0xF];
    }
    buf[12] = ' ';
    buf[13] = 'E'; buf[14] = 'R'; buf[15] = 'R'; buf[16] = '=';
    for (int i = 0; i < 8; i++) {
        buf[17+i] = hex[(error >> ((7-i)*4)) & 0xF];
    }
    buf[25] = '\r'; buf[26] = '\n'; buf[27] = '\0';
    serial_puts(buf);
}

/* Macro that generates an exception stub with 2-letter code */
#define EXCEPTION_STUB(name, c1, c2)                            \
    void name(void);                                            \
    __asm__(                                                    \
        ".global " #name "\n"                                   \
        #name ":\n"                                             \
        "  movl $0xB8000+(78*2), %eax\n"                       \
        "  movw $" #c1 ", (%eax)\n"   /* char en col 78 */     \
        "  movl $0xB8000+(79*2), %eax\n"                       \
        "  movw $" #c2 ", (%eax)\n"   /* char en col 79 */     \
        "  cli\n"                                               \
        "1: hlt\n"                                              \
        "  jmp 1b\n"                                            \
    )

/* 0x00 Divide by Zero      → 'DZ' */
EXCEPTION_STUB(exc_divide_error,        0x4C44, 0x4C5A);
/* 0x06 Invalid Opcode      → 'UD' */
EXCEPTION_STUB(exc_invalid_opcode,      0x4C55, 0x4C44);
/* 0x08 Double Fault        → 'DF' */
EXCEPTION_STUB(exc_double_fault,        0x4C44, 0x4C46);
/* 0x0D General Protection  → 'GP' */
/* Custom handler to log via serial for debugging */
void exc_gpf(void);
__asm__(
    ".global exc_gpf\n"
    "exc_gpf:\n"
    /* notify serial/logging */
    "  call gpf_report\n"
    /* dibujar 'GP' como antes */
    "  movl $0xB8000+(78*2), %eax\n"
    "  movw $0x4C47, (%eax)\n"
    "  movl $0xB8000+(79*2), %eax\n"
    "  movw $0x4C50, (%eax)\n"
    "  cli\n"
    "1: hlt\n"
    "  jmp 1b\n"
);

/* 0x0E Page Fault → 'PF' - custom handler below */
/* Resto de excepciones     → 'EX' */
EXCEPTION_STUB(exc_generic,             0x4C45, 0x4C58);

/* Definición manual del handler de page fault para mostrar CR2 */
void exc_page_fault(void);
__asm__(
    ".global exc_page_fault\n"
    "exc_page_fault:\n"
    /* guardar dirección fallida en variable */
    "  movl %cr2, %eax\n"
    "  push %eax\n"
    "  call pf_report\n"
    "  add $4, %esp\n"
    /* dibujar 'PF' como antes */
    "  movl $0xB8000+(78*2), %eax\n"
    "  movw $0x4C50, (%eax)\n"
    "  movl $0xB8000+(79*2), %eax\n"
    "  movw $0x4C46, (%eax)\n"
    "  cli\n"
    "1: hlt\n"
    "  jmp 1b\n"
);

/* ═══════════════════════════════════════════════════════
 * Handlers de IRQ hardware (0x20-0x2F)
 * Todos envian EOI y retornan. El timer (IRQ0) no hace
 * nada extra por ahora.
 * ═══════════════════════════════════════════════════════ */

/* IRQ generico: solo EOI y IRET */
void irq_generic_handler(void);

/* Entrada de syscall (vector 0x30) definida en syscall.c */
extern void syscall_entry(void);

__asm__(
    ".global irq_generic_handler\n"
    "irq_generic_handler:\n"
    "  pusha\n"
    "  movb $0x20, %al\n"
    "  outb %al, $0x20\n"       /* EOI al master */
    "  outb %al, $0xA0\n"       /* EOI al slave  */
    "  popa\n"
    "  iret\n"
);

/*
 * IRQ0 - Timer + Scheduler
 *
 * Este es el corazon del context switch. Cada tick del timer (18.2Hz):
 *   1. Guarda TODOS los registros del thread interrumpido en su kernel stack
 *   2. Llama a scheduler_tick(ctx) con el puntero al contexto guardado
 *   3. scheduler_tick retorna el ctx del proximo thread a ejecutar
 *   4. Cambia ESP al nuevo contexto y hace IRET
 *
 * El stack frame que armamos es compatible con cpu_context_t:
 *   [esp]    gs, fs, es, ds        (4 x 4 = 16 bytes)
 *   [esp+16] edi,esi,ebp,esp_dummy,ebx,edx,ecx,eax  (pusha = 32 bytes)
 *   [esp+48] int_no, err_code      (8 bytes, ficticios)
 *   [esp+56] eip, cs, eflags       (empujados por la CPU = 12 bytes)
 *   Total: 68 bytes = sizeof(cpu_context_t) sin user_esp/user_ss
 *
 * NOTA: Este handler corre en Ring 0 (kernel), por eso la CPU NO empuja
 * user_esp ni user_ss. El campo user_esp/user_ss de cpu_context_t solo
 * se usa para threads de Ring 3 (futura implementacion).
 */
void irq0_timer_handler(void);
extern cpu_context_t* scheduler_tick(cpu_context_t* ctx);
__asm__(
    ".global irq0_timer_handler\n"
    "irq0_timer_handler:\n"

    /*
     * Construimos en el stack del thread interrumpido un frame
     * identico a cpu_context_t. El orden de push determina el
     * layout en memoria — debe coincidir EXACTAMENTE con la
     * estructura cpu_context_t en process.h:
     *
     *  offset 0  : gs             <- tope del stack (menor direccion)
     *  offset 4  : fs
     *  offset 8  : es
     *  offset 12 : ds
     *  offset 16 : edi  \  
     *  offset 20 : esi   |
     *  offset 24 : ebp   | pusha/popa
     *  offset 28 : esp_d |
     *  offset 32 : ebx   |
     *  offset 36 : edx   |
     *  offset 40 : ecx   |
     *  offset 44 : eax  /
     *  offset 48 : int_no  (forzamos 0x20)
     *  offset 52 : err_code (forzamos 0)
     *  offset 56 : eip  <- CPU empezo a pushear aqui
     *  offset 60 : cs
     *  offset 64 : eflags
     *
     * PUSH en x86: ESP = ESP-4, luego escribe en [ESP]. Por tanto
     * el PRIMER push queda en la direccion MAS ALTA y es lo ULTIMO
     * en desapilarse. Pusheamos de 'fondo hacia arriba'.
     */

    /* [1] err_code y int_no — la CPU ya empezo a pushear eip/cs/eflags
     *     antes de saltar aqui. Ahora pusheamos ficticios encima. */
    "  pushl $0\n"        /* err_code */
    "  pushl $0x20\n"     /* int_no   */

    /* [2] PUSHA empuja: eax ecx edx ebx esp ebp esi edi
     *     (en ese orden, edi queda en la dir mas baja = tope del bloque) */
    "  pusha\n"

    /* [3] Segmentos: pusheamos gs primero para que quede al FONDO del
     *     bloque de segmentos (mayor dir), ds al tope (menor dir) */
    "  pushl %gs\n"
    "  pushl %fs\n"
    "  pushl %es\n"
    "  pushl %ds\n"

    /* [4] Apuntar segmentos al kernel data segment */
    "  movw $0x10, %ax\n"
    "  movw %ax,   %ds\n"
    "  movw %ax,   %es\n"
    "  movw %ax,   %fs\n"
    "  movw %ax,   %gs\n"

    /* [5] EOI: avisar al PIC que recibimos el tick ANTES de llamar
     *     al scheduler, para que el siguiente tick pueda encolarse */
    "  movb $0x20, %al\n"
    "  outb %al,   $0x20\n"

    /* [6] Llamar a scheduler_tick(cpu_context_t* ctx)
     *
     *     TRAMPA: 'push %esp' en x86 empuja ESP-4 (el valor DESPUES
     *     del push). Para pasar el ESP actual correcto usamos EBX
     *     como intermediario — EBX ya fue salvado por pusha, podemos
     *     sobreescribirlo temporalmente. */
    "  movl %esp, %ebx\n"   /* EBX = puntero al cpu_context_t (= ESP actual) */
    "  pushl %ebx\n"        /* argumento para scheduler_tick(ctx) */
    "  call scheduler_tick\n"
    "  addl $4, %esp\n"     /* limpiar argumento */

    /* [7] EAX = retorno de scheduler_tick = nuevo cpu_context_t*
     *     Cargarlo como nuevo ESP para restaurar el nuevo thread */
    "  movl %eax, %esp\n"

    /* [8] Restaurar segmentos del nuevo thread (orden inverso al push) */
    "  popl %ds\n"
    "  popl %es\n"
    "  popl %fs\n"
    "  popl %gs\n"

    /* [9] Restaurar registros generales del nuevo thread */
    "  popa\n"

    /* [10] Descartar int_no y err_code ficticios */
    "  addl $8, %esp\n"

    /* [11] IRET consume eip/cs/eflags -> nuevo thread continua */
    "  iret\n"
);

/* IRQ1 - Teclado PS/2: leer el scancode del buffer (limpia el buffer del
 * controlador PS/2) y enviar EOI. Sin esto el controlador bloquea el bus. */
/* IRQ1 handler simply reads scancode to clear PS/2 buffer */

void irq1_keyboard_handler(void);
__asm__(
    ".global irq1_keyboard_handler\n"
    "irq1_keyboard_handler:\n"
    "  pusha\n"
    /* leer scancode para vaciar buffer */
    "  inb  $0x60, %al\n"
    "  movb $0x20, %al\n"
    "  outb %al, $0x20\n"       /* EOI */
    "  popa\n"
    "  iret\n"
);

/* ═══════════════════════════════════════════════════════
 * Helpers IDT
 * ═══════════════════════════════════════════════════════ */
static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags)
{
    idt[num].offset_low  = base & 0xFFFF;
    idt[num].offset_high = (base >> 16) & 0xFFFF;
    idt[num].selector    = sel;
    idt[num].zero        = 0;
    idt[num].type_attr   = flags;
}

/* ═══════════════════════════════════════════════════════
 * idt_init() - Inicializar IDT + PIC
 * ═══════════════════════════════════════════════════════ */
void idt_init(void)
{
    int i;

    /* 1. Remapear el PIC ANTES de cargar la IDT — si llegara una IRQ
     *    con el PIC sin remapear y la IDT ya cargada, el vector 0x08
     *    (Double Fault) recibiria el tick del timer → triple fault */
    pic_remap();

    /* 2. Configurar puntero IDT */
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base  = (uint32_t)&idt;

    /* 3. Rellenar todas las entradas con el handler generico de excepcion */
    for (i = 0; i < 256; i++) {
        idt_set_gate((uint8_t)i, (uint32_t)exc_generic, 0x08, 0x8E);
    }

    /* 4. Excepciones de CPU especificas (0x00-0x1F) */
    idt_set_gate(0x00, (uint32_t)exc_divide_error,   0x08, 0x8E);
    idt_set_gate(0x06, (uint32_t)exc_invalid_opcode, 0x08, 0x8E);
    idt_set_gate(0x08, (uint32_t)exc_double_fault,   0x08, 0x8E);
    idt_set_gate(0x0D, (uint32_t)exc_gpf,            0x08, 0x8E);
    idt_set_gate(0x0E, (uint32_t)exc_page_fault,     0x08, 0x8E);

    /* 5. IRQ hardware remapeados (0x20-0x2F) */
    idt_set_gate(0x20, (uint32_t)irq0_timer_handler,    0x08, 0x8E); /* Timer    */
    idt_set_gate(0x21, (uint32_t)irq1_keyboard_handler, 0x08, 0x8E); /* Teclado  */
    for (i = 0x22; i <= 0x2F; i++) {
        idt_set_gate((uint8_t)i, (uint32_t)irq_generic_handler, 0x08, 0x8E);
    }
    /* 6. Vector 0x30: syscalls desde Ring 3 (DPL=3) */
    idt_set_gate(0x30, (uint32_t)syscall_entry, 0x08, 0xEE); /* present, ring3, 32-bit interrupt gate */

    /* 6. Cargar IDT */
    __asm__ volatile("lidt %0" :: "m"(idtp));

    /* El PIC tiene todas las IRQ enmascaradas (0xFF).
     * Desenmascaramos solo IRQ0 (timer) para que el sistema siga vivo.
     * IRQ1 (teclado/mouse PS/2) se habilitara cuando el driver lo necesite. */
    outb_idt(PIC1_DATA, 0xFE);  /* 0xFE = 11111110 → solo IRQ0 activa */
    outb_idt(PIC2_DATA, 0xFF);  /* Slave completo enmascarado */
}
