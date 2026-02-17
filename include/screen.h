#ifndef _SCREEN_H
#define _SCREEN_H

#include "types.h"

/* VGA text mode screen interface */

/* Screen dimensions */
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

/* VGA colors */
enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_YELLOW = 14,  /* Alias for LIGHT_BROWN */
    VGA_COLOR_WHITE = 15,
};

/* Initialize screen */
void screen_init(void);

/* Clear screen */
void screen_clear(void);

/* Write string to screen */
void screen_write(const char* str);

/* Write string with newline */
void screen_writeln(const char* str);

/* Set screen color */
void screen_set_color(enum vga_color fg, enum vga_color bg);

#endif /* _SCREEN_H */
