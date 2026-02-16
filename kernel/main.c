#include "kernel.h"
#include "screen.h"
#include "hal.h"
#include "multiboot.h"

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

    /* Display welcome message */
    screen_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    screen_writeln("System Operative Edit v0.1");
    screen_writeln("Based on ReactOS");
    screen_writeln("");

    /* Initialize HAL */
    screen_write("Initializing HAL... ");
    hal_init();
    screen_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    screen_writeln("OK");

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

    /* Halt system */
    screen_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    screen_writeln("");
    screen_writeln("System halted.");

    kernel_halt();
}
