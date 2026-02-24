#ifndef _GUI_H
#define _GUI_H

#include <types.h>
#include <video/vga/vga.h>   /* for VGA_COLOR_* constants */

/* colores reutilizan los definidos en <vga.h> */
#define GUI_COLOR_DESKTOP      VGA_COLOR_CYAN
#define GUI_COLOR_TITLEBAR     VGA_COLOR_BLUE
#define GUI_COLOR_TITLEBAR_TXT VGA_COLOR_WHITE
#define GUI_COLOR_WINDOW_BG    VGA_COLOR_LIGHT_GRAY
#define GUI_COLOR_BORDER       VGA_COLOR_DARK_GRAY
#define GUI_COLOR_BUTTON_BG    VGA_COLOR_LIGHT_GRAY
#define GUI_COLOR_BUTTON_TXT   VGA_COLOR_BLACK
#define GUI_COLOR_SHADOW       VGA_COLOR_DARK_GRAY

/* dimensiones de fuente 8x8 usadas en el GUI */
#define FONT_WIDTH   8
#define FONT_HEIGHT  8

#define TITLEBAR_HEIGHT 10
#define BORDER_SIZE      2
#define BUTTON_W        16
#define BUTTON_H         8

/* ventana simplificada compartida entre kernel y usuario */
typedef struct _GUI_WINDOW {
    int x, y;
    int w, h;
    const char* title;    /* puntero en espacio de usuario */
    int visible;
} GUI_WINDOW;

/* Mouse event entregado por la cola del servicio GUI */
typedef struct _GUI_MOUSE_EVENT {
    int x;
    int y;
    int buttons;  /* bit0 = izquierdo, bit1 = derecho, etc. */
} GUI_MOUSE_EVENT;

/* Prototipos de funciones proporcionadas por el servicio GUI (kernel).
   Algunos de estos son invocados directamente en el kernel, otros son
   accedidos a través de syscalls desde user-space. */

/* inicializar subsistema GUI (cursor, estructuras) */
void GuiInit(void);

/* cola de eventos de mouse */
void GuiQueueMouseEvent(int x, int y, int buttons);
int  GuiGetMouseEvent(GUI_MOUSE_EVENT* out);

/* primitivas de dibujo */
void GuiDrawDesktop(void);
void GuiDrawTaskbar(void);
void GuiDrawWindow(const GUI_WINDOW* win);
void GuiDrawWindowText(const GUI_WINDOW* win, int rx, int ry, const char* txt, UCHAR fg);
/* otros elementos gráficos útiles */
void GuiDrawButton(int x, int y, int w, int h, const char* label, int pressed);
void GuiDrawHLine(int x, int y, int len, UCHAR color);
void GuiDrawVLine(int x, int y, int len, UCHAR color);
#endif /* _GUI_H */