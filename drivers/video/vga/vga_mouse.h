/*
 * vga_mouse.h - Mouse/Cursor API para VGA
 */
#ifndef _VGA_MOUSE_H
#define _VGA_MOUSE_H

#include "vga.h"

#define MOUSE_WIDTH  8
#define MOUSE_HEIGHT 12

/* Estado del mouse */
typedef struct _MOUSE_STATE {
    INT  x, y;          /* Posicion actual */
    INT  buttons;       /* Bits: 0=izq, 1=der, 2=medio */
    INT  visible;       /* Cursor visible */
} MOUSE_STATE;

/* Inicializar el mouse (PS/2) */
VOID MouseInit(VOID);

/* Leer estado del mouse */
VOID MouseRead(MOUSE_STATE* state);

/* Dibujar cursor en pantalla */
VOID MouseDraw(INT x, INT y);

/* Borrar cursor (restaurar fondo) */
VOID MouseErase(INT x, INT y);

/* Obtener estado actual */
MOUSE_STATE* MouseGetState(VOID);

#endif
