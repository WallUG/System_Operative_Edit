#include "kernel.h"
#include "screen.h"
#include "hal.h"
#include "multiboot.h"

/* Forward declarations for driver types */
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
    cpu_disable_interrupts();
    while (1) {
        cpu_halt();
    }
}

void kernel_main(uint32_t magic, struct multiboot_info* mboot)
{
    /* Initialize screen */
    screen_init();
    screen_clear();

    /* Display Universidad de Guayaquil branded welcome message */
    screen_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    screen_writeln("================================================================================");
    screen_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    screen_writeln("                       UNIVERSIDAD DE GUAYAQUIL");
    screen_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    screen_writeln("                    System Operative Edit v0.1");
    screen_writeln("                    Edicion Universidad de Guayaquil");
    screen_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    screen_writeln("                         Based on ReactOS");
    screen_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    screen_writeln("================================================================================");
    screen_writeln("");

    /* Initialize HAL */
    screen_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    screen_write("Initializing HAL... ");
    hal_init();
    screen_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    screen_writeln("OK");

    /* Initialize I/O Manager */
    screen_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    screen_write("Initializing I/O Manager... ");
    NTSTATUS status = IoInitSystem();
    if (NT_SUCCESS(status)) {
        screen_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        screen_writeln("OK");
    } else {
        screen_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        screen_writeln("FAILED");
    }

    /* Load VGA driver */
    screen_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    screen_write("Loading VGA driver... ");
    UNICODE_STRING driverName;
    /* String: "\Driver\VGA" = 11 characters */
    static const WCHAR driverNameStr[] = {'\\', 'D', 'r', 'i', 'v', 'e', 'r', '\\', 'V', 'G', 'A', 0};
    driverName.Buffer = (PWCHAR)driverNameStr;
    driverName.Length = 11 * sizeof(WCHAR);      /* 11 characters (not counting null) */
    driverName.MaximumLength = 12 * sizeof(WCHAR); /* 11 + 1 for null terminator */
    
    status = IoCreateDriver(&driverName, VgaDriverEntry);
    if (NT_SUCCESS(status)) {
        screen_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        screen_writeln("OK");
        
        /* Initialize HAL display */
        screen_write("Initializing HAL Display... ");
        status = HalInitializeDisplay();
        if (NT_SUCCESS(status)) {
            screen_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
            screen_writeln("OK");
        } else {
            screen_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
            screen_writeln("FAILED");
        }
    } else {
        screen_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        screen_writeln("FAILED");
    }

    /* Verify multiboot */
    screen_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    screen_write("Multiboot magic: ");
    if (magic == MULTIBOOT_BOOTLOADER_MAGIC) {
        screen_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        screen_writeln("Valid");
    } else {
        screen_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        screen_writeln("Invalid");
    }

    /* Display memory info if available */
    screen_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    if (mboot && (mboot->flags & MULTIBOOT_MEMORY_INFO)) {
        screen_write("Memory lower: ");
        screen_writeln("Available");
        screen_write("Memory upper: ");
        screen_writeln("Available");
    }

    /* Success message */
    screen_writeln("");
    screen_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    screen_writeln("Kernel initialized successfully!");
    
    /* Test VGA graphics mode if driver loaded successfully */
    if (NT_SUCCESS(status)) {
        screen_writeln("");
        screen_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
        screen_writeln("Testing VGA graphics mode...");
        screen_writeln("Drawing demo pattern in 5 seconds...");
        
        /* Simple delay (very rough - CPU speed dependent) */
        /* TODO: Implement proper timer-based delay mechanism */
        /* This is a temporary placeholder until timer support is added */
        for (volatile int i = 0; i < 50000000; i++);
        
        /* Draw VGA demo */
        VgaDrawDemo();
        
        /* Note: After this, we're in graphics mode and can't use text mode anymore */
        /* In a real implementation, we'd have a way to switch back or draw text in graphics mode */
    }

    /* Halt system */
    screen_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    screen_writeln("");
    screen_writeln("System halted.");

    kernel_halt();
}
