#include "kernel.h"
#include "screen.h"
#include "hal.h"
#include "multiboot.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "proc/process.h"
#include "proc/scheduler.h"

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
    /* Initialize GDT and IDT FIRST - prevents triple fault */
    gdt_init();
    idt_init();

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

    if (!NT_SUCCESS(IoCreateDriver(&driverName, VgaDriverEntry))) {
        kernel_panic("Failed to load VGA driver");
    }
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

    /* Gestor de procesos: crear proceso idle (PID 0) */
    proc_init();
    screen_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    screen_writeln("[OK] Gestor de procesos inicializado");

    if (!NT_SUCCESS(HalInitializeDisplay())) {
        kernel_panic("Failed to initialize display");
    }

    /* Inicializar mouse ANTES de crear el proceso GUI */
    MouseInit();

    /* Crear el proceso GUI como proceso de kernel (Ring 0 por ahora).
     * GuiMainLoop() sera el entry point del thread GUI.
     * El scheduler lo ejecutara de forma cooperativa con el idle. */
    proc_create_kernel("gui_server", GuiMainLoop);
    screen_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    screen_writeln("[OK] Proceso gui_server creado (PID 1)");

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
