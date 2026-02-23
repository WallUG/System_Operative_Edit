#include <gui.h>
#include "../drivers/video/vga/vga.h"
#include "../drivers/video/vga/vga_font.h"
#include "../drivers/video/vga/vga_cursor.h"
#include "../drivers/input/ps2mouse.h"
#include "../interrupt/syscall.h"  /* contiene dispatcher */

/* el contador de ticks se mantiene en syscall.c */
extern uint32_t get_tick_count(void);

#include <types.h>
#include <stddef.h>
#include <string.h>

/* mantener algunos estados internos */
static int g_start_pressed = 0;

/* cola de eventos de mouse */
#define MOUSE_QUEUE_SIZE 32
static GUI_MOUSE_EVENT g_mouse_queue[MOUSE_QUEUE_SIZE];
static int g_mouse_qhead = 0;
static int g_mouse_qtail = 0;

void GuiInit(void)
{
    /* Inicializar cursor (llamado desde MouseInit) */
    CursorInit();
}

void GuiQueueMouseEvent(int x, int y, int buttons)
{
    int next = (g_mouse_qtail + 1) % MOUSE_QUEUE_SIZE;
    if (next == g_mouse_qhead) {
        /* cola llena, descartar */
        return;
    }
    g_mouse_queue[g_mouse_qtail].x = x;
    g_mouse_queue[g_mouse_qtail].y = y;
    g_mouse_queue[g_mouse_qtail].buttons = buttons;
    g_mouse_qtail = next;

    /* manejar interacción básica con la barra de tareas */
    /* coordenadas del botón "Start" (2,471) tamaño 36x8 */
    if (x >= 2 && x < 2 + 36 && y >= 471 && y < 471 + 8) {
        if (buttons & 1) {
            if (!g_start_pressed) {
                g_start_pressed = 1;
                GuiDrawTaskbar();
            }
        } else {
            if (g_start_pressed) {
                g_start_pressed = 0;
                GuiDrawTaskbar();
                /* al liberar, abrimos "consola" simple */
                int cx = 100, cy = 100;
                VgaFillRect(cx, cy, 200, 80, GUI_COLOR_WINDOW_BG);
                VgaDrawString(cx + 4, cy + 4, "Consola (sin teclado)", VGA_COLOR_BLACK, GUI_COLOR_WINDOW_BG);
            }
        }
    }
}

int GuiGetMouseEvent(GUI_MOUSE_EVENT* evt)
{
    if (g_mouse_qhead == g_mouse_qtail) {
        return -1;
    }
    *evt = g_mouse_queue[g_mouse_qhead];
    g_mouse_qhead = (g_mouse_qhead + 1) % MOUSE_QUEUE_SIZE;
    return 0;
}

/* rutinas de dibujo compartidas (antiguo usuario) */
static void DrawHLine(int x, int y, int len, UCHAR color)
{
    VgaFillRect(x, y, len, 1, color);
}
static void DrawVLine(int x, int y, int len, UCHAR color)
{
    VgaFillRect(x, y, 1, len, color);
}

void GuiDrawDesktop(void)
{
    VgaFillRect(0, 0, 640, 470, GUI_COLOR_DESKTOP);
    VgaDrawString(240, 210, "Universidad de Guayaquil", VGA_COLOR_WHITE, GUI_COLOR_DESKTOP);
    VgaDrawString(260, 222, "System Operative Edit", VGA_COLOR_LIGHT_GRAY, GUI_COLOR_DESKTOP);
}

void GuiDrawTaskbar(void)
{
    int y = 470;
    VgaFillRect(0, y, 640, 10, GUI_COLOR_BUTTON_BG);
    DrawHLine(0, y, 640, VGA_COLOR_WHITE);
    /* dibujar boton Start con estado presionado automatico */
    int bx = 2, by = y + 1, bw = 36, bh = 8;
    VgaFillRect(bx, by, bw, bh, GUI_COLOR_BUTTON_BG);
    if (!g_start_pressed) {
        DrawHLine(bx,       by,       bw,   VGA_COLOR_WHITE);
        DrawVLine(bx,       by,       bh,   VGA_COLOR_WHITE);
        DrawHLine(bx,       by+bh-1,   bw,   GUI_COLOR_SHADOW);
        DrawVLine(bx+bw-1,   by,       bh,   GUI_COLOR_SHADOW);
    } else {
        DrawHLine(bx,       by,       bw,   GUI_COLOR_SHADOW);
        DrawVLine(bx,       by,       bh,   GUI_COLOR_SHADOW);
        DrawHLine(bx,       by+bh-1,   bw,   VGA_COLOR_WHITE);
        DrawVLine(bx+bw-1,   by,       bh,   VGA_COLOR_WHITE);
    }
    VgaDrawString(bx+8, by, "Start", VGA_COLOR_BLACK, GUI_COLOR_BUTTON_BG);
    /* reloj */
    uint32_t ticks = get_tick_count();
    uint32_t secs = ticks / 100; /* asumiendo timer 100Hz */
    uint32_t mins = (secs / 60) % 60;
    uint32_t hours = (secs / 3600) % 24;
    char buf[6];
    buf[0] = '0' + (hours / 10) % 10;
    buf[1] = '0' + (hours % 10);
    buf[2] = ':';
    buf[3] = '0' + (mins / 10) % 10;
    buf[4] = '0' + (mins % 10);
    buf[5] = '\0';
    VgaDrawString(600, y + 1, buf, VGA_COLOR_BLACK, GUI_COLOR_BUTTON_BG);
}

void GuiDrawWindow(const GUI_WINDOW* win)
{
    if (!win || !win->visible) return;
    int x = win->x, y = win->y, w = win->w, h = win->h;
    VgaFillRect(x+3, y+3, w, h, GUI_COLOR_SHADOW);
    VgaFillRect(x, y, w, h, GUI_COLOR_WINDOW_BG);
    VgaFillRect(x, y, w, TITLEBAR_HEIGHT, GUI_COLOR_TITLEBAR);
    if (win->title) {
        VgaDrawString(x + 4, y + 1, win->title, GUI_COLOR_TITLEBAR_TXT, GUI_COLOR_TITLEBAR);
    }
    /* boton cerrar fijo */
    DrawHLine(x,     y,     w,   VGA_COLOR_WHITE);
    DrawVLine(x,     y,     h,   VGA_COLOR_WHITE);
    DrawHLine(x,     y+h-1, w,   GUI_COLOR_SHADOW);
    DrawVLine(x+w-1, y,     h,   GUI_COLOR_SHADOW);
    DrawHLine(x, y + TITLEBAR_HEIGHT, w, GUI_COLOR_BORDER);
}

void GuiDrawWindowText(const GUI_WINDOW* win, int rx, int ry, const char* txt, UCHAR fg)
{
    if (!win || !txt) return;
    VgaDrawString(win->x + BORDER_SIZE + rx,
                  win->y + TITLEBAR_HEIGHT + BORDER_SIZE + ry,
                  txt, fg, GUI_COLOR_WINDOW_BG);
}
