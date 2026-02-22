#include "kernel.h"
#include "screen.h"
#include "hal.h"
#include "multiboot.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "proc/process.h"
#include "proc/scheduler.h"
#include "interrupt/tss.h"
#include "interrupt/syscall.h"
#include "boot_splash.h"
#include <kstdlib.h>

/* entrada de mouse PS/2 (antes en vga_mouse.h) */
#include "../drivers/input/ps2mouse.h"

/* Forward declarations */
#include "interrupt/gdt.h"   /* gdt_init + gdt_install_tss */
extern void idt_init(void);

/* Driver declarations */
typedef int32_t NTSTATUS;
typedef uint32_t ULONG;
typedef unsigned short WCHAR;
typedef WCHAR *PWCHAR;
typedef struct _UNICODE_STRING {
    uint16_t Length;
    uint16_t MaximumLength;
    PWCHAR Buffer;
} UNICODE_STRING, *PUNICODE_STRING;
typedef NTSTATUS (*PDRIVER_INITIALIZE)(void*, PUNICODE_STRING);
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)

/* Driver functions */
extern NTSTATUS IoInitSystem(void);
extern NTSTATUS IoCreateDriver(PUNICODE_STRING, PDRIVER_INITIALIZE);
extern NTSTATUS VgaDriverEntry(void*, PUNICODE_STRING);
extern NTSTATUS HalInitializeDisplay(void);
extern void VgaDrawDemo(void);
/* GuiMainLoop ahora corre en espacio de usuario */
/* extern void MouseInit(void);  moved to ps2mouse.h */

void kernel_panic(const char* message)
{
    screen_set_color(VGA_COLOR_WHITE, VGA_COLOR_RED);
    screen_writeln("KERNEL PANIC!");
    screen_write("Error: ");
    screen_writeln(message);
    kernel_halt();
}

void kernel_halt(void)
{
    __asm__ volatile("cli; hlt");
    while(1);
}



void kernel_main(uint32_t magic, multiboot_info_t* mbi)
{
    /*
     * GDT e IDT PRIMERO — obligatorio antes de cualquier otra cosa.
     *
     * KernelShowBootSplash() usaba acceso directo a puertos VGA (sin INT 0x10),
     * por lo que NO necesita estar antes de la GDT/IDT.
     * Llamarla antes causaba triple fault: la IDT de GRUB tiene limite 0 en
     * el contexto del kernel, cualquier interrupcion durante la animacion
     * (timer, teclado) genera un page fault sin handler -> reset.
     */
    /*
     * HEAP PRIMERO — antes de cualquier llamada a malloc().
     * heap_init() calcula el inicio del heap usando el simbolo _kernel_end
     * del linker script, garantizando que el heap nunca solape con el
     * kernel image (antes HEAP_START == 0x100000 == inicio del kernel).
     */
    heap_init();

    /* iniciar la consola serial para facilitar la depuración en QEMU */
    extern void serial_init(void);
    extern void serial_puts(const char*);
    serial_init();
    serial_puts("[boot] serial init\r\n");

    gdt_init();
    idt_init();

    /*
     * ANIMACION DE BOOT — ahora seguro despues de GDT/IDT.
     * Las interrupciones siguen desactivadas (cli en boot.asm),
     * por lo que la animacion corre sin interferencia del timer/teclado.
     * La boot animation usa puertos I/O directos, no INT 0x10.
     */
    KernelShowBootSplash();
	
	/* Pausa pre-VGA con interrupciones deshabilitadas */
    for (volatile int _delay = 0; _delay < 0xC000000; _delay++)
        __asm__ volatile("nop");

    /* NOTA: interrupciones se habilitan DESPUES del scheduler_init(),
     * no aqui. El texto de boot corre con CLI para evitar IRQ espurias
     * durante la inicializacion. */
    /* __asm__ volatile("sti"); <- movido al final */

    /* Clear screen and setup colors */
    screen_clear();
    screen_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);

    /* Mostrar primera PF ocurrida antes de inicialización completa */
    extern volatile uint32_t g_last_pf_addr;
    if (g_last_pf_addr) {
        screen_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        screen_write("EARLY PF @ 0x");
        uint32_t v = g_last_pf_addr;
        char buf[9];
        for (int i = 0; i < 8; i++)
            buf[i] = "0123456789ABCDEF"[(v >> ((7 - i) * 4)) & 0xF];
        buf[8] = '\0';
        screen_writeln(buf);
        /* Detener el boot para facilitar depuración */
        kernel_halt();
    }

    /* Header UG */
    screen_writeln("================================================================================");
    screen_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    screen_writeln("                       UNIVERSIDAD DE GUAYAQUIL");
    screen_writeln("                    System Operative Edit v0.1");
    screen_writeln("                  Edicion Universidad de Guayaquil");
    screen_writeln("                         Based on ReactOS");
    screen_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    screen_writeln("================================================================================");
    screen_writeln("");

    /* Check multiboot */
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        if (magic != 0x2BADB002) {
            kernel_panic("Invalid multiboot magic number");
        }
        mbi = 0;
    }

    /* Mostrar estado de inicializacion en modo texto ANTES de pasar a grafico */
    screen_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    screen_writeln("[OK] GDT/IDT inicializados");

    hal_init();
    screen_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    screen_writeln("[OK] HAL inicializado");

    UNICODE_STRING driverName = {0, 0, NULL};
    if (!NT_SUCCESS(IoInitSystem())) {
        kernel_panic("Failed to initialize I/O system");
    }
    screen_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    screen_writeln("[OK] Sistema de I/O inicializado");

    

    /* ── v0.2: Subsistemas de memoria ──
     * Deben iniciarse ANTES del driver VGA para que el PMM no
     * asigne frames que ya usa el kernel image o el framebuffer. */

    /* PMM: inicializar con la RAM reportada por multiboot */
    {
        uint32_t mem_upper_kb = 32768;   /* 32MB por defecto */
        if (mbi && (mbi->flags & 0x01)) {
            mem_upper_kb = mbi->mem_upper;
        }
        pmm_init(mem_upper_kb);
    }
    screen_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    screen_writeln("[OK] PMM inicializado");

    /* VMM: crear page directory del kernel + activar paginacion
     * Identity-map 0x00000000-0x07FFFFFF (cubre kernel + VGA 0xA0000) */
    vmm_init();
    screen_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    screen_writeln("[OK] VMM + paginacion activada");

    /* TSS: necesario para Ring 3 → Ring 0 en syscalls e interrupciones.
     * Debe inicializarse DESPUES de gdt_init() y vmm_init().
     * gdt_install_tss() extiende la GDT con el descriptor del TSS en entrada 5. */
        /* TSS: necesario para Ring 3 → Ring 0 en syscalls e interrupciones.
     * Debe inicializarse DESPUES de gdt_init() y vmm_init().
     * gdt_install_tss() extiende la GDT con el descriptor del TSS en entrada 5.
     */
    tss_init();
    screen_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    screen_writeln("[OK] TSS inicializado (selector 0x28)");

    /* Gestor de procesos: crear proceso idle (PID 0) */
    proc_init();
    /* Inicializar ESP0 para el thread idle recién creado */
    {
        thread_t* idle = proc_current_thread();
        if (idle) {
            extern void tss_set_esp0(uint32_t esp0);
            tss_set_esp0(idle->kernel_stack_top);
        }
    }
    screen_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    screen_writeln("[OK] Gestor de procesos inicializado");

    if (!NT_SUCCESS(HalInitializeDisplay())) {
        kernel_panic("Failed to initialize display");
    }

    /* volver a chequear si un PF ocurrió justo antes de cambiar a vídeo */
    {
        extern volatile uint32_t g_last_pf_addr;
        if (g_last_pf_addr) {
            screen_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
            screen_write("LATE PF @ 0x");
            uint32_t v = g_last_pf_addr;
            char buf[9];
            for (int i = 0; i < 8; i++)
                buf[i] = "0123456789ABCDEF"[(v >> ((7 - i) * 4)) & 0xF];
            buf[8] = '\0';
            screen_writeln(buf);
            kernel_halt();
        }
    }

    MouseInit();

    
    screen_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    screen_writeln("");
    screen_write("    Iniciando modo grafico  [");
    screen_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    screen_write("####################");
    screen_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    screen_writeln("] 100%");
    screen_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    screen_writeln("");
    screen_writeln("    Cargando interfaz grafica...");
    screen_writeln("    (La pantalla cambiara a modo grafico en un momento)");

    /* Pausa para que el usuario pueda leer el mensaje */
    for (volatile int _delay = 0; _delay < 0xC000000; _delay++)
        __asm__ volatile("nop");

    /* Ahora si: inicializar el driver VGA (cambia a modo grafico 0x12) */
    if (!NT_SUCCESS(IoCreateDriver(&driverName, VgaDriverEntry))) {
        kernel_panic("Failed to load VGA driver");
    }
    /* NOTA: screen_writeln() ya no funciona aqui (modo grafico activo) */

    /* dump bytes from both potential buffers so we know qué memoria
       está interpretando el emulador. */
    serial_puts("[VGA] buffer @A0000:");
    for (int i = 0; i < 16; i++) {
        uint8_t b = *((volatile uint8_t*)0xA0000 + i);
        char buf[5] = "  0";
        const char *hex = "0123456789ABCDEF";
        buf[1] = hex[(b >> 4) & 0xF];
        buf[2] = hex[b & 0xF];
        buf[3] = ' ';
        buf[4] = '\0';
        serial_puts(buf);
    }
    serial_puts("\r\n[VGA] buffer @B8000:");
    for (int i = 0; i < 16; i++) {
        uint8_t b = *((volatile uint8_t*)0xB8000 + i);
        char buf[5] = "  0";
        const char *hex = "0123456789ABCDEF";
        buf[1] = hex[(b >> 4) & 0xF];
        buf[2] = hex[b & 0xF];
        buf[3] = ' ';
        buf[4] = '\0';
        serial_puts(buf);
    }
    serial_puts("\r\n");

    /* gui_server como proceso de usuario Ring 3 */
    {
        serial_puts("[boot] creating gui_server\r\n");
        extern uint8_t _user_start[];
        extern uint8_t _user_end[];
        extern void user_entry(void);

        uint32_t code_phys = (uint32_t)_user_start;
        uint32_t code_size = (uint32_t)(_user_end - _user_start);
        if (!proc_create_user("gui_server", code_phys, code_size,
                              (uint32_t)user_entry)) {
            kernel_panic("Failed to create GUI user process");
        }
    }
	
    /* Inicializar el scheduler.
     * proc_create_kernel() ya llamo scheduler_add_thread() internamente
     * para el thread del gui_server. El idle lo anade scheduler_init(). */
    serial_puts("[boot] scheduler init\r\n");
    scheduler_init();

    /* NOTA: No llamar screen_writeln aqui - VGA ya esta en modo grafico */

    /*
     * Idle loop.
     *
     * En este punto kernel_main ES el thread idle del scheduler:
     * - proc_init() creo un thread idle con TID=1 cuyo saved_context
     *   apunta a kernel_idle() en un stack separado.
     * - scheduler_init() puso ese thread idle en la cola con state=RUNNING.
     * - El primer tick del timer guardara el contexto de ESTE punto
     *   (kernel_main/hlt) en saved_context del idle, y luego cambiara
     *   al gui_server (quantum se agota en SCHEDULER_QUANTUM ticks).
     *
     * NO usar 'cli' aqui — el scheduler necesita las interrupciones.
     * 'sti' garantiza que IF=1 antes del primer hlt.
     * 'hlt' suspende hasta el proximo tick — 0% CPU entre ticks.
     */
    __asm__ volatile("sti");
    while (1) {
        __asm__ volatile("hlt");
    }
}