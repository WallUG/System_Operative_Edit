/*
 * vga_font.h - Bitmap Font API para modo grafico VGA
 */
#ifndef _VGA_FONT_H
#define _VGA_FONT_H

#include "vga.h"

/* Dimensiones de cada caracter en la fuente 8x8 */
#define FONT_WIDTH   8
#define FONT_HEIGHT  8

/* Dibujar un caracter en coordenadas (x,y) con color de frente y fondo */
VOID VgaDrawChar(int32_t x, int32_t y, char c, UCHAR fg, UCHAR bg);

/* Dibujar una cadena de texto */
VOID VgaDrawString(int32_t x, int32_t y, const char* str, UCHAR fg, UCHAR bg);

/* Dibujar numero entero */
VOID VgaDrawInt(int32_t x, int32_t y, int32_t num, UCHAR fg, UCHAR bg);

#endif
