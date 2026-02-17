#include "kernel.h"
#include "screen.h"
#include "hal.h"
#include "multiboot.h"

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
    
    /* Display UG branding header */
    screen_writeln("================================================================================");
    screen_writeln("                       UNIVERSIDAD DE GUAYAQUIL");
    screen_writeln("                    System Operative Edit v0.1");
    screen_writeln("                    Edicion Universidad de Guayaquil");
    screen_writeln("                         Based on ReactOS");
    screen_writeln("================================================================================");
    screen_writeln("");
    
    /* Check multiboot */
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        kernel_panic("Invalid multiboot magic number");
    }
    
    screen_writeln("[OK] GDT initialized");
    screen_writeln("[OK] IDT initialized");
    screen_writeln("[OK] Multiboot verified");
    screen_writeln("");
    
    /* Initialize HAL */
    screen_write("Initializing HAL... ");
    hal_init();
    screen_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    screen_writeln("[OK]");
    screen_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    
    /* Initialize I/O System */
    screen_write("Initializing I/O System... ");
    if (!NT_SUCCESS(IoInitSystem())) {
        kernel_panic("Failed to initialize I/O system");
    }
    screen_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    screen_writeln("[OK]");
    screen_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    
    /* Initialize VGA driver */
    screen_write("Loading VGA driver... ");
    UNICODE_STRING driverName = {0, 0, NULL};
    if (!NT_SUCCESS(IoCreateDriver(&driverName, VgaDriverEntry))) {
        kernel_panic("Failed to load VGA driver");
    }
    screen_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    screen_writeln("[OK]");
    screen_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    
    /* Initialize display */
    screen_write("Initializing display... ");
    if (!NT_SUCCESS(HalInitializeDisplay())) {
        kernel_panic("Failed to initialize display");
    }
    screen_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    screen_writeln("[OK]");
    screen_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    
    screen_writeln("");
    screen_writeln("Kernel initialization complete!");
    screen_writeln("System ready.");
    screen_writeln("");
    
    /* Demo: Draw some patterns */
    screen_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    screen_writeln("Running VGA demo...");
    VgaDrawDemo();
    
    /* Idle loop */
    screen_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    screen_writeln("Entering idle state...");
    
    while(1) {
        __asm__ volatile("hlt");
    }
}