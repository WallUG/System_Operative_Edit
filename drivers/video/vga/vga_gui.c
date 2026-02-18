/*
 * vga_gui.c — GUI tipo Windows que usa syscalls para dibujar
 *
 * v0.4 (Ring 3 real):
 *   - El proceso GUI corre en Ring 3 con su propio espacio de direcciones.
 *   - TODA comunicacion con el kernel ocurre via INT 0x30.
 *   - NO se llama a ninguna función del kernel directamente.
 *   - MouseRead() fue eliminada: el estado del mouse se obtiene por valor
 *     con sys_get_mouse_state(), que copia al buffer de usuario.
 *   - MouseDraw/MouseErase fueron eliminadas: el cursor se dibuja via
 *     syscalls SYS_DRAW_PIXEL desde el espacio de usuario.
 *
 * INVARIANTE: Este archivo NO debe incluir ningun header del kernel
 * excepto libsys.h y los propios de vga_gui.
 */
#include "vga_gui.h"
#include <libsys.h>

/* Colores VGA — deben definirse ANTES de cualquier uso */
#define VGA_COLOR_BLACK         0
#define VGA_COLOR_BLUE          1
#define VGA_COLOR_GREEN         2
#define VGA_COLOR_CYAN          3
#define VGA_COLOR_RED           4
#define VGA_COLOR_MAGENTA       5
#define VGA_COLOR_BROWN         6
#define VGA_COLOR_LIGHT_GRAY    7
#define VGA_COLOR_DARK_GRAY     8
#define VGA_COLOR_LIGHT_BLUE    9
#define VGA_COLOR_LIGHT_GREEN   10
#define VGA_COLOR_LIGHT_CYAN    11
#define VGA_COLOR_LIGHT_RED     12
#define VGA_COLOR_LIGHT_MAGENTA 13
#define VGA_COLOR_YELLOW        14
#define VGA_COLOR_WHITE         15

/* ── Cursor del mouse en modo Ring 3 ────────────────────────────────
 * En Ring 3 no podemos llamar a MouseDraw/MouseErase del driver VGA
 * (esas funciones usan VgaPutPixel directo). En su lugar redibujamos
 * el cursor con syscalls SYS_DRAW_PIXEL.
 *
 * El fondo del cursor se guarda en un buffer local (espacio de usuario).
 * Los colores son indices VGA iguales a los del driver.
 */

#define MOUSE_W  8
#define MOUSE_H  12

/* Bitmap del cursor: 1=blanco, 2=negro(borde), 0=transparente */
static const unsigned char s_cursor[MOUSE_H][MOUSE_W] = {
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

/* Fondo guardado bajo el cursor (buffer de usuario) */
static unsigned char s_cursor_bg[MOUSE_H * MOUSE_W];
static int s_saved_x = -1;
static int s_saved_y = -1;

/* Dibujar cursor (no usa función del kernel: usa sys_draw_pixel) */
static void cursor_draw(int x, int y)
{
    int r, c;
    /* Guardar fondo: en Ring 3 no tenemos acceso al shadow framebuffer
     * del driver VGA. Inicializamos el fondo al color del desktop (azul)
     * para que el erase funcione aunque no tengamos lectura de VGA. */
    for (r = 0; r < MOUSE_H; r++)
        for (c = 0; c < MOUSE_W; c++)
            s_cursor_bg[r * MOUSE_W + c] = GUI_COLOR_DESKTOP;
    s_saved_x = x;
    s_saved_y = y;

    for (r = 0; r < MOUSE_H; r++) {
        for (c = 0; c < MOUSE_W; c++) {
            unsigned char p = s_cursor[r][c];
            if (p == 1) sys_draw_pixel(x + c, y + r, VGA_COLOR_WHITE);
            else if (p == 2) sys_draw_pixel(x + c, y + r, VGA_COLOR_BLACK);
        }
    }
}

/* Borrar cursor restaurando el fondo */
static void cursor_erase(int x, int y)
{
    int r, c;
    if (x < 0 || y < 0) return;
    for (r = 0; r < MOUSE_H; r++)
        for (c = 0; c < MOUSE_W; c++)
            sys_draw_pixel(x + c, y + r, s_cursor_bg[r * MOUSE_W + c]);
}

/* Colores VGA: ya definidos al inicio del archivo — ver bloque superior */

/* ── Helpers internos ─────────────────────────────────────────────────── */

/* strlen sin libc */
static int gui_strlen(const char* s)
{
    int n = 0;
    while (s && *s++) n++;
    return n;
}

VOID GuiDrawDesktop(VOID)
{
    sys_fill_rect(0, 0, 640, 470, GUI_COLOR_DESKTOP);
    sys_draw_string(230, 208, "Universidad de Guayaquil",
                    VGA_COLOR_WHITE, GUI_COLOR_DESKTOP);
    sys_draw_string(252, 220, "System Operative Edit v0.3",
                    VGA_COLOR_LIGHT_GRAY, GUI_COLOR_DESKTOP);
    sys_draw_string(268, 232, "Syscalls activas - INT 0x30",
                    VGA_COLOR_YELLOW, GUI_COLOR_DESKTOP);
}

VOID GuiDrawHLine(INT x, INT y, INT len, UCHAR color)
{
    sys_fill_rect(x, y, len, 1, color);
}

VOID GuiDrawVLine(INT x, INT y, INT len, UCHAR color)
{
    sys_fill_rect(x, y, 1, len, color);
}

VOID GuiDrawButton(INT x, INT y, INT w, INT h, const char* label, INT pressed)
{
    sys_fill_rect(x, y, w, h, GUI_COLOR_BUTTON_BG);

    if (!pressed) {
        GuiDrawHLine(x,     y,     w,   VGA_COLOR_WHITE);
        GuiDrawVLine(x,     y,     h,   VGA_COLOR_WHITE);
        GuiDrawHLine(x,     y+h-1, w,   GUI_COLOR_SHADOW);
        GuiDrawVLine(x+w-1, y,     h,   GUI_COLOR_SHADOW);
    } else {
        GuiDrawHLine(x,     y,     w,   GUI_COLOR_SHADOW);
        GuiDrawVLine(x,     y,     h,   GUI_COLOR_SHADOW);
        GuiDrawHLine(x,     y+h-1, w,   VGA_COLOR_WHITE);
        GuiDrawVLine(x+w-1, y,     h,   VGA_COLOR_WHITE);
    }

    if (label) {
        int tx = x + (w - gui_strlen(label) * FONT_WIDTH) / 2;
        int ty = y + (h - FONT_HEIGHT) / 2;
        sys_draw_string(tx, ty, label, GUI_COLOR_BUTTON_TXT, GUI_COLOR_BUTTON_BG);
    }
}

VOID GuiDrawWindow(WINDOW* win)
{
    if (!win || !win->visible) return;

    INT x = win->x, y = win->y, w = win->w, h = win->h;

    /* Sombra */
    sys_fill_rect(x+3, y+3, w, h, GUI_COLOR_SHADOW);

    /* Fondo */
    sys_fill_rect(x, y, w, h, GUI_COLOR_WINDOW_BG);

    /* Barra de titulo */
    sys_fill_rect(x, y, w, TITLEBAR_HEIGHT, GUI_COLOR_TITLEBAR);

    if (win->title) {
        sys_draw_string(x + 4, y + 1, win->title,
                        GUI_COLOR_TITLEBAR_TXT, GUI_COLOR_TITLEBAR);
    }

    /* Boton cierre */
    GuiDrawButton(x + w - BUTTON_W - 2, y + 1, BUTTON_W, BUTTON_H, "X", 0);

    /* Borde */
    GuiDrawHLine(x,     y,     w,   VGA_COLOR_WHITE);
    GuiDrawVLine(x,     y,     h,   VGA_COLOR_WHITE);
    GuiDrawHLine(x,     y+h-1, w,   GUI_COLOR_SHADOW);
    GuiDrawVLine(x+w-1, y,     h,   GUI_COLOR_SHADOW);
    GuiDrawHLine(x, y + TITLEBAR_HEIGHT, w, GUI_COLOR_BORDER);
}

VOID GuiDrawWindowText(WINDOW* win, INT rx, INT ry, const char* txt, UCHAR fg)
{
    if (!win || !txt) return;
    sys_draw_string(win->x + BORDER_SIZE + rx,
                    win->y + TITLEBAR_HEIGHT + BORDER_SIZE + ry,
                    txt, fg, GUI_COLOR_WINDOW_BG);
}

VOID GuiDrawTaskbar(VOID)
{
    INT y = 470;
    sys_fill_rect(0, y, 640, 10, GUI_COLOR_BUTTON_BG);
    GuiDrawHLine(0, y, 640, VGA_COLOR_WHITE);
    GuiDrawButton(2, y + 1, 36, 8, "Start", 0);
    sys_draw_string(600, y + 1, "00:00", VGA_COLOR_BLACK, GUI_COLOR_BUTTON_BG);
}

INT GuiHitTest(INT mx, INT my, INT x, INT y, INT w, INT h)
{
    return (mx >= x && mx < x+w && my >= y && my < y+h);
}

/* ── Dibujar contador de ticks en la barra de tareas ─────────────────── */
static void draw_tick_counter(void)
{
    uint32_t ticks = sys_get_tick();
    char buf[12];
    int i = 10;
    buf[11] = '\0';
    buf[10] = '\0';
    if (ticks == 0) {
        buf[--i] = '0';
    } else {
        while (ticks > 0 && i > 0) {
            buf[--i] = '0' + (ticks % 10);
            ticks /= 10;
        }
    }
    sys_draw_string(540, 471, buf + i, VGA_COLOR_BLACK, GUI_COLOR_BUTTON_BG);
}

VOID GuiMainLoop(VOID)
{
    INT prev_x = 320, prev_y = 240;
    uint32_t prev_tick = 0;

    WINDOW welcome = {
        150, 80, 340, 130,
        "Sistema Operativo UG - v0.4 Ring 3",
        1
    };

    /* Dibujar escena inicial via syscalls */
    GuiDrawDesktop();
    GuiDrawTaskbar();
    GuiDrawWindow(&welcome);
    GuiDrawWindowText(&welcome, 10, 10,
        "Sistema iniciado correctamente.", VGA_COLOR_BLACK);
    GuiDrawWindowText(&welcome, 10, 22,
        "v0.4: GUI en Ring 3 real!", VGA_COLOR_BLACK);
    GuiDrawWindowText(&welcome, 10, 34,
        "Scheduler Round-Robin activo.", VGA_COLOR_DARK_GRAY);
    GuiDrawWindowText(&welcome, 10, 46,
        "Mouse via syscall por valor.", VGA_COLOR_DARK_GRAY);
    GuiDrawWindowText(&welcome, 10, 58,
        "Punteros de usuario validados.", VGA_COLOR_DARK_GRAY);
    GuiDrawButton(welcome.x + 130, welcome.y + 100, 60, 14, "Aceptar", 0);

    /* Cursor inicial (usando syscalls, sin acceso directo a VGA) */
    cursor_draw(320, 240);

    while (1) {
        /* Ceder CPU al idle entre frames */
        sys_yield();

        /* Actualizar mouse: obtener estado por VALOR (seguro desde Ring 3)
         * El kernel copia los datos al buffer local 'ms' en espacio de usuario. */
        SYS_MOUSE ms;
        if (sys_get_mouse_state(&ms) == 0) {
            if (ms.x != prev_x || ms.y != prev_y) {
                cursor_erase(prev_x, prev_y);
                cursor_draw(ms.x, ms.y);
                prev_x = ms.x;
                prev_y = ms.y;
            }
        }

        /* Actualizar contador de ticks en taskbar cada 18 ticks (~1s) */
        uint32_t tick = sys_get_tick();
        if (tick - prev_tick >= 18) {
            draw_tick_counter();
            prev_tick = tick;
        }
    }
}
