/*
 * vga_gui.c - Implementacion de la GUI tipo Windows
 */
#include "vga_gui.h"

VOID GuiDrawDesktop(VOID)
{
    /* Fondo del escritorio */
    VgaFillRect(0, 0, 640, 470, GUI_COLOR_DESKTOP);

    /* Logo UG en el escritorio */
    VgaDrawString(240, 210, "Universidad de Guayaquil", VGA_COLOR_WHITE, GUI_COLOR_DESKTOP);
    VgaDrawString(260, 222, "System Operative Edit", VGA_COLOR_LIGHT_GRAY, GUI_COLOR_DESKTOP);
}

VOID GuiDrawHLine(INT x, INT y, INT len, UCHAR color)
{
    VgaFillRect(x, y, len, 1, color);
}

VOID GuiDrawVLine(INT x, INT y, INT len, UCHAR color)
{
    VgaFillRect(x, y, 1, len, color);
}

VOID GuiDrawButton(INT x, INT y, INT w, INT h, const char* label, INT pressed)
{
    /* Fondo del boton */
    VgaFillRect(x, y, w, h, GUI_COLOR_BUTTON_BG);

    if (!pressed) {
        /* Borde 3D: blanco arriba-izquierda, oscuro abajo-derecha */
        GuiDrawHLine(x,       y,       w,   VGA_COLOR_WHITE);
        GuiDrawVLine(x,       y,       h,   VGA_COLOR_WHITE);
        GuiDrawHLine(x,       y+h-1,   w,   GUI_COLOR_SHADOW);
        GuiDrawVLine(x+w-1,   y,       h,   GUI_COLOR_SHADOW);
    } else {
        /* Invertido cuando presionado */
        GuiDrawHLine(x,       y,       w,   GUI_COLOR_SHADOW);
        GuiDrawVLine(x,       y,       h,   GUI_COLOR_SHADOW);
        GuiDrawHLine(x,       y+h-1,   w,   VGA_COLOR_WHITE);
        GuiDrawVLine(x+w-1,   y,       h,   VGA_COLOR_WHITE);
    }

    /* Texto centrado */
    if (label) {
        int tx = x + (w - (int)(/* strlen */ ({
            int _l=0; const char*_s=label; while(*_s++){_l++;} _l;
        }) * FONT_WIDTH)) / 2;
        int ty = y + (h - FONT_HEIGHT) / 2;
        VgaDrawString(tx, ty, label, GUI_COLOR_BUTTON_TXT, GUI_COLOR_BUTTON_BG);
    }
}

VOID GuiDrawWindow(WINDOW* win)
{
    if (!win || !win->visible) return;

    INT x = win->x, y = win->y, w = win->w, h = win->h;

    /* Sombra */
    VgaFillRect(x+3, y+3, w, h, GUI_COLOR_SHADOW);

    /* Fondo de la ventana */
    VgaFillRect(x, y, w, h, GUI_COLOR_WINDOW_BG);

    /* Barra de titulo */
    VgaFillRect(x, y, w, TITLEBAR_HEIGHT, GUI_COLOR_TITLEBAR);

    /* Titulo */
    if (win->title) {
        VgaDrawString(x + 4, y + 1, win->title, GUI_COLOR_TITLEBAR_TXT, GUI_COLOR_TITLEBAR);
    }

    /* Boton de cierre [X] en la barra de titulo */
    GuiDrawButton(x + w - BUTTON_W - 2, y + 1, BUTTON_W, BUTTON_H, "X", 0);

    /* Borde externo de la ventana */
    GuiDrawHLine(x,     y,     w,   VGA_COLOR_WHITE);      /* top */
    GuiDrawVLine(x,     y,     h,   VGA_COLOR_WHITE);      /* left */
    GuiDrawHLine(x,     y+h-1, w,   GUI_COLOR_SHADOW);     /* bottom */
    GuiDrawVLine(x+w-1, y,     h,   GUI_COLOR_SHADOW);     /* right */

    /* Separador bajo la barra de titulo */
    GuiDrawHLine(x, y + TITLEBAR_HEIGHT, w, GUI_COLOR_BORDER);
}

VOID GuiDrawWindowText(WINDOW* win, INT rx, INT ry, const char* txt, UCHAR fg)
{
    if (!win || !txt) return;
    VgaDrawString(win->x + BORDER_SIZE + rx,
                  win->y + TITLEBAR_HEIGHT + BORDER_SIZE + ry,
                  txt, fg, GUI_COLOR_WINDOW_BG);
}

VOID GuiDrawTaskbar(VOID)
{
    INT y = 470;

    /* Fondo de la barra */
    VgaFillRect(0, y, 640, 10, GUI_COLOR_BUTTON_BG);

    /* Borde superior */
    GuiDrawHLine(0, y, 640, VGA_COLOR_WHITE);

    /* Boton Start */
    GuiDrawButton(2, y + 1, 36, 8, "Start", 0);

    /* Reloj (placeholder) */
    VgaDrawString(600, y + 1, "00:00", VGA_COLOR_BLACK, GUI_COLOR_BUTTON_BG);
}

INT GuiHitTest(INT mx, INT my, INT x, INT y, INT w, INT h)
{
    return (mx >= x && mx < x+w && my >= y && my < y+h);
}

VOID GuiMainLoop(VOID)
{
    MOUSE_STATE* ms;
    INT prev_x = -1, prev_y = -1;

    /* Ventana de bienvenida */
    WINDOW welcome = { 150, 80, 340, 120, "Bienvenido - System Operative Edit", 1 };

    /* Dibujar escena inicial */
    GuiDrawDesktop();
    GuiDrawTaskbar();
    GuiDrawWindow(&welcome);
    GuiDrawWindowText(&welcome, 10, 10, "Sistema iniciado correctamente.", VGA_COLOR_BLACK);
    GuiDrawWindowText(&welcome, 10, 22, "Universidad de Guayaquil v0.1", VGA_COLOR_DARK_GRAY);
    GuiDrawButton(welcome.x + 130, welcome.y + 90, 60, 14, "Aceptar", 0);

    /* Dibujar cursor inicial */
    MouseDraw(320, 240);

    /* Loop principal */
    while (1) {
        ms = MouseGetState();
        MouseRead(ms);

        /* Solo redibujar si el mouse se movio */
        if (ms->x != prev_x || ms->y != prev_y) {
            MouseErase(prev_x, prev_y);
            MouseDraw(ms->x, ms->y);
            prev_x = ms->x;
            prev_y = ms->y;
        }

        __asm__ volatile("hlt");
    }
}
