#include "vga_cursor.h"
#include "vga.h"    /* VgaGetPixel, VgaPutPixel */

/* Fondo del cursor, ancho*alto bytes */
static UCHAR g_cursor_bg[CURSOR_WIDTH * CURSOR_HEIGHT];
static int32_t g_cursor_saved_x = -1;
static int32_t g_cursor_saved_y = -1;

/* Bitmap estático de la flecha (1=blanco, 2=negro, 0=transparente) */
static const unsigned char cursor_bitmap[CURSOR_HEIGHT][CURSOR_WIDTH] = {
    {1,0,0,0,0,0,0,0},
    {1,1,0,0,0,0,0,0},
    {1,2,1,0,0,0,0,0},
    {1,2,2,1,0,0,0,0},
    {1,2,2,2,1,0,0,0},
    {1,2,2,2,2,1,0,0},
    {1,2,2,2,2,2,1,0},
    {1,2,2,2,1,1,0,0},
    {1,2,1,2,1,0,0,0},
    {1,1,0,1,2,1,0,0},
    {0,0,0,1,2,1,0,0},
    {0,0,0,0,1,1,0,0},
};

void CursorInit(void) {
    /* nada por ahora; los buffers se llenan en CursorDraw */
    g_cursor_saved_x = -1;
    g_cursor_saved_y = -1;
}

static int g_cursor_invert = 0;

void CursorToggleInvert(void) {
    g_cursor_invert = !g_cursor_invert;
}

void CursorDraw(int32_t x, int32_t y) {
    int row, col;
    /* guardar fondo real */
    for (row = 0; row < CURSOR_HEIGHT; row++) {
        for (col = 0; col < CURSOR_WIDTH; col++) {
            g_cursor_bg[row * CURSOR_WIDTH + col] =
                VgaGetPixel(x + col, y + row);
        }
    }
    g_cursor_saved_x = x;
    g_cursor_saved_y = y;

    /* dibujar flecha */
    for (row = 0; row < CURSOR_HEIGHT; row++) {
        for (col = 0; col < CURSOR_WIDTH; col++) {
            unsigned char pixel = cursor_bitmap[row][col];
            if (pixel == 1) {
                VgaPutPixel(x + col, y + row, g_cursor_invert ? VGA_COLOR_BLACK : VGA_COLOR_WHITE);
            } else if (pixel == 2) {
                VgaPutPixel(x + col, y + row, g_cursor_invert ? VGA_COLOR_WHITE : VGA_COLOR_BLACK);
            }
        }
    }
}

void CursorErase(int32_t x, int32_t y) {
    int row, col;
    if (x < 0 || y < 0) return;
    /* no restauramos si está dentro de la barra de tareas */
    if (y >= 470) return;
    for (row = 0; row < CURSOR_HEIGHT; row++) {
        for (col = 0; col < CURSOR_WIDTH; col++) {
            UCHAR bg = g_cursor_bg[row * CURSOR_WIDTH + col];
            VgaPutPixel(x + col, y + row, bg);
        }
    }
}
