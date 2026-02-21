/*
 * vga_cursor.h - API de cursor para VGA (dibujado/restaurado)
 *
 * Este módulo sólo conoce el framebuffer VGA; no gestiona el hardware PS/2
 * ni el estado del ratón. La posición/estado se obtiene de un driver de
 * entrada independiente (ps2mouse).
 */
#ifndef _VGA_CURSOR_H
#define _VGA_CURSOR_H

#include <types.h>   /* INT, UCHAR */

#define CURSOR_WIDTH  8
#define CURSOR_HEIGHT 12

/* Inicializa internamente los buffers del cursor (no necesario si se
 * usa MouseInit/MouseRead antes de dibujar, pero puede llamarse de forma
 * explícita si se quiere). */
void CursorInit(void);

/* Dibujar cursor en la posición indicada. Guarda el contenido previo. */
void CursorDraw(INT x, INT y);

/* Borrar cursor previamente dibujado en la posición especificada. */
void CursorErase(INT x, INT y);

#endif /* _VGA_CURSOR_H */
