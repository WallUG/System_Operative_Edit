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

/* Forward declarations */
extern void gdt_init(void);
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
extern void GuiMainLoop(void);
extern void MouseInit(void);

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
    gdt_init();
    idt_init();

    /*
     * ANIMACION DE BOOT — ahora seguro despues de GDT/IDT.
     * Las interrupciones siguen desactivadas (cli en boot.asm),
     * por lo que la animacion corre sin interferencia del timer/teclado.
     * La boot animation usa puertos I/O directos, no INT 0x10.
     */
    KernelShowBootSplash();

    /* Enable interrupts after IDT is set */
    __asm__ volatile("sti");

    /* Clear screen and setup colors */
    screen_clear();
    screen_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);

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

    /* Desactivar interrupciones durante la init del VGA.
     * IRQ 12 (mouse) e IRQ 1 (teclado) pueden llegar mientras
     * VgaInitializeDevice programa los registros del AC/Sequencer,
     * corrompiendo el flip-flop del Attribute Controller.
     * El STI al final de este bloque las reactiva. */
    __asm__ volatile("cli");
    if (!NT_SUCCESS(IoCreateDriver(&driverName, VgaDriverEntry))) {
        __asm__ volatile("sti");
        kernel_panic("Failed to load VGA driver");
    }
    __asm__ volatile("sti");
    screen_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    screen_writeln("[OK] Driver VGA 640x480x16 cargado");

    /* Barrita de progreso en texto */
    screen_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    screen_write("    Iniciando modo grafico  [");
    screen_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    screen_write("####################");
    screen_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    screen_writeln("] 100%");
    screen_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    screen_writeln("");
    screen_writeln("    Cargando interfaz grafica...");

    /* Pequena espera activa SIN delay — solo un loop de I/O inocuo
     * para que el texto sea visible (el CPU lo ejecuta en microsegundos
     * pero nos da al menos un frame en QEMU) */
    for (volatile int _i = 0; _i < 0x4FFFF; _i++) {
        __asm__ volatile("nop");
    }

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
    tss_init();
    screen_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    screen_writeln("[OK] TSS inicializado (selector 0x28)");

    /* Gestor de procesos: crear proceso idle (PID 0) */
    proc_init();
    screen_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    screen_writeln("[OK] Gestor de procesos inicializado");

    __asm__ volatile("cli");
    if (!NT_SUCCESS(HalInitializeDisplay())) {
        __asm__ volatile("sti");
        kernel_panic("Failed to initialize display");
    }
    __asm__ volatile("sti");

    /* Inicializar mouse ANTES de crear el proceso GUI */
    MouseInit();

    /* Crear el proceso GUI como proceso de kernel (Ring 0 por ahora).
     * GuiMainLoop() sera el entry point del thread GUI.
     * El scheduler lo ejecutara de forma cooperativa con el idle. */
    /* Syscalls: registrar INT 0x30 en la IDT con DPL=3
     * Debe hacerse DESPUES de idt_init() porque usa idt_set_gate_pub(). */
    syscall_init();
    screen_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    screen_writeln("[OK] Syscalls INT 0x30 registradas (DPL=3)");

    /*
     * v0.4: Crear el proceso GUI en Ring 3 real.
     * proc_create_user_gui() mapea el kernel con PTE_USER, crea un page
     * directory propio y construye el frame de IRET con CS=0x1B (Ring 3).
     * El GUI corre con CPL=3 y solo puede llamar al kernel via INT 0x30.
     */
    proc_create_user_gui("gui_server", GuiMainLoop);
    screen_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    screen_writeln("[OK] Proceso gui_server creado en Ring 3 (PID 1)");
    screen_writeln("     -> CS=0x1B SS=0x23 CPL=3 / syscalls INT 0x30");

    /* Inicializar el scheduler.
     * proc_create_kernel() ya llamo scheduler_add_thread() internamente
     * para el thread del gui_server. El idle lo anade scheduler_init(). */
    scheduler_init();

    screen_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    screen_writeln("[OK] Scheduler v0.2 activo — Round-Robin");
    screen_writeln("");
    screen_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    screen_writeln("    Transfiriendo control al scheduler...");

    /* Pausa para que el texto sea visible */
    for (volatile int _i = 0; _i < 0x2FFFF; _i++) __asm__ volatile("nop");

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
