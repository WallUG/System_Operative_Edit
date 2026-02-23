/*
 * ps2mouse.h - Driver de mouse PS/2 (entrada abstraida)
 *
 * Este módulo implementa la comunicación con el dispositivo PS/2 y mantiene
 * un registro simple de la posición/estado del cursor. No conoce nada de VGA
 * ni dibuja el cursor; eso queda en el subsistema de video.
 */
#ifndef _PS2MOUSE_H
#define _PS2MOUSE_H

#include <types.h>   /* proporciona int32_t, uint8_t, etc. */

#define MOUSE_WIDTH  8
#define MOUSE_HEIGHT 12

/* Estado del mouse compartido con el resto del sistema */
typedef struct _MOUSE_STATE {
    int32_t x, y;       /* Posicion actual */
    int32_t buttons;    /* Bits: 0=izq, 1=der, 2=medio */
    int32_t visible;    /* Cursor visible */
} MOUSE_STATE;

/* Inicializar el controlador PS/2 y habilitar IRQ de mouse */
void MouseInit(void);

/* Handler auxiliar llamado desde el IRQ1 del PIC. Se encarga de distinguir
   entre byte de teclado y paquete de mouse mediante el bit de estado. */
void ps2_irq(void);

/* Actualizar el estado interno leyendo un paquete PS/2.
 * El parámetro "state" es opcional y se rellena con la copia actual.
 */
void MouseRead(MOUSE_STATE* state);

/* Devuelve puntero al estado interno (no copiar!) */
MOUSE_STATE* MouseGetState(void);

#endif /* _PS2MOUSE_H */
