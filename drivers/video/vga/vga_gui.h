/*
 * vga_gui.h - API de ventanas y controles GUI
 *
 * OBSOLETO: el servicio GUI ahora se encuentra en kernel/gui/gui.c y
 * expone sus primitivas a través de syscalls. Este archivo se conserva
 * solo para referencia histórica y ya no se compila.
 */
#ifndef _VGA_GUI_H
#define _VGA_GUI_H

#include "vga.h"
#include "vga_font.h"
#include <input/ps2mouse.h>
#include "vga_cursor.h"

/* Colores del tema */
#define GUI_COLOR_DESKTOP      VGA_COLOR_CYAN
#define GUI_COLOR_TITLEBAR     VGA_COLOR_BLUE
#define GUI_COLOR_TITLEBAR_TXT VGA_COLOR_WHITE
#define GUI_COLOR_WINDOW_BG    VGA_COLOR_LIGHT_GRAY
#define GUI_COLOR_BORDER       VGA_COLOR_DARK_GRAY
#define GUI_COLOR_BUTTON_BG    VGA_COLOR_LIGHT_GRAY
#define GUI_COLOR_BUTTON_TXT   VGA_COLOR_BLACK
#define GUI_COLOR_SHADOW       VGA_COLOR_DARK_GRAY

#define TITLEBAR_HEIGHT 10
#define BORDER_SIZE      2
#define BUTTON_W        16
#define BUTTON_H         8

/* Descriptor de una ventana */
typedef struct _WINDOW {
    INT     x, y;           /* Posicion en pantalla */
    INT     w, h;           /* Ancho y alto total */
    const char* title;      /* Titulo */
    INT     visible;        /* Visible */
} WINDOW;

/* Dibujar escritorio (fondo) */
VOID GuiDrawDesktop(VOID);

/* Dibujar una ventana completa con barra de titulo */
VOID GuiDrawWindow(WINDOW* win);

/* Dibujar un boton */
VOID GuiDrawButton(INT x, INT y, INT w, INT h, const char* label, INT pressed);

/* Dibujar barra de tareas en la parte inferior */
VOID GuiDrawTaskbar(VOID);

/* Dibujar texto dentro de una ventana (coordenadas relativas) */
VOID GuiDrawWindowText(WINDOW* win, INT rx, INT ry, const char* txt, UCHAR fg);

/* Dibujar linea horizontal con estilo */
VOID GuiDrawHLine(INT x, INT y, INT len, UCHAR color);

/* Dibujar linea vertical con estilo */
VOID GuiDrawVLine(INT x, INT y, INT len, UCHAR color);

/* Verificar si el mouse esta sobre un area */
INT GuiHitTest(INT mx, INT my, INT x, INT y, INT w, INT h);

/* Loop principal de la GUI */
VOID GuiMainLoop(VOID);

#endif
