#include "screen.h"

/* VGA text mode buffer address */
static uint16_t* const VGA_BUFFER = (uint16_t*)0xB8000;

/* Current cursor position */
static size_t screen_row = 0;
static size_t screen_col = 0;

/* Current color */
static uint8_t screen_color = 0x07; /* Light grey on black */

/* Helper function to create VGA entry */
static inline uint16_t vga_entry(unsigned char uc, uint8_t color)
{
    return (uint16_t)uc | (uint16_t)color << 8;
}

/* Helper function to create VGA color */
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg)
{
    return fg | bg << 4;
}

void screen_init(void)
{
    screen_row = 0;
    screen_col = 0;
    screen_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
}

void screen_clear(void)
{
    for (size_t y = 0; y < SCREEN_HEIGHT; y++) {
        for (size_t x = 0; x < SCREEN_WIDTH; x++) {
            const size_t index = y * SCREEN_WIDTH + x;
            VGA_BUFFER[index] = vga_entry(' ', screen_color);
        }
    }
    screen_row = 0;
    screen_col = 0;
}

void screen_set_color(enum vga_color fg, enum vga_color bg)
{
    screen_color = vga_entry_color(fg, bg);
}

/* Helper function to scroll screen */
static void screen_scroll(void)
{
    /* Move all lines up */
    for (size_t y = 0; y < SCREEN_HEIGHT - 1; y++) {
        for (size_t x = 0; x < SCREEN_WIDTH; x++) {
            const size_t src_index = (y + 1) * SCREEN_WIDTH + x;
            const size_t dst_index = y * SCREEN_WIDTH + x;
            VGA_BUFFER[dst_index] = VGA_BUFFER[src_index];
        }
    }

    /* Clear last line */
    for (size_t x = 0; x < SCREEN_WIDTH; x++) {
        const size_t index = (SCREEN_HEIGHT - 1) * SCREEN_WIDTH + x;
        VGA_BUFFER[index] = vga_entry(' ', screen_color);
    }

    screen_row = SCREEN_HEIGHT - 1;
}

/* Helper function to put character at position */
static void screen_putchar(char c)
{
    if (c == '\n') {
        screen_col = 0;
        screen_row++;
    } else {
        const size_t index = screen_row * SCREEN_WIDTH + screen_col;
        VGA_BUFFER[index] = vga_entry(c, screen_color);
        screen_col++;
    }

    if (screen_col >= SCREEN_WIDTH) {
        screen_col = 0;
        screen_row++;
    }

    if (screen_row >= SCREEN_HEIGHT) {
        screen_scroll();
    }
}

void screen_write(const char* str)
{
    if (!str) return;

    while (*str) {
        screen_putchar(*str);
        str++;
    }
}

void screen_writeln(const char* str)
{
    screen_write(str);
    screen_putchar('\n');
}
