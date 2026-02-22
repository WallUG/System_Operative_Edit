#include <libsys.h>
#include <types.h>
#include <video/vga/vga.h>    /* colores VGA y prototipos de VGA driver */
#include <video/vga/vga_font.h> /* VgaDrawString prototype and font constants */

/* Macro para colocar literales en la sección .user.rodata */
#define USTR(str) (__extension__({ \
    static const char __u[] __attribute__((section(".user.rodata"))) = str; \
    __u; }))

/* helpers para declarar funciones/datos de usuario en secciones propias */
#define UCODE __attribute__((section(".user")))
#define UDATA __attribute__((section(".user.data")))   /* datos writables del usuario */
#define URODATA __attribute__((section(".user.rodata")))

/* Nota: las funciones de usuario se colocan en la sección .user vía
   atributo __attribute__((section(".user"))) en user_entry. */

/* colores y constantes replicadas de vga_gui.h */
#define GUI_COLOR_DESKTOP      VGA_COLOR_CYAN

/* dimensiones de fuente 8x8 usadas en el GUI */
#define FONT_WIDTH   8
#define FONT_HEIGHT  8

/* colores locales reutilizan los valores definidos en <vga.h> */
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

/* descriptor simplificado de ventana */
typedef struct _WINDOW {
    int x, y;
    int w, h;
    const char* title;
    int visible;
} WINDOW;

/* prototipos internos */
static void CursorDraw(int x, int y);
static void CursorErase(int x, int y);
static int GuiHitTest(int mx, int my, int x, int y, int w, int h);

static void GuiDrawDesktop(void);
static void GuiDrawHLine(int x, int y, int len, unsigned char color);
static void GuiDrawVLine(int x, int y, int len, unsigned char color);
static void GuiDrawButton(int x, int y, int w, int h, const char* label, int pressed);
static void GuiDrawWindow(WINDOW* win);
static void GuiDrawWindowText(WINDOW* win, int rx, int ry, const char* txt, unsigned char fg);
static void GuiDrawTaskbar(void);

/* cursor bitmap and buffers */
#define CURSOR_WIDTH 8
#define CURSOR_HEIGHT 12
/* background buffer lives in user data section so accesses from Ring3 work */
static unsigned char g_cursor_bg[CURSOR_WIDTH * CURSOR_HEIGHT] UDATA;
static int g_cursor_saved_x UDATA = -1, g_cursor_saved_y UDATA = -1;
/* bitmap is constant read-only data; put in user rodata section */
static const unsigned char cursor_bitmap[CURSOR_HEIGHT][CURSOR_WIDTH] URODATA = {
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

/* implementaciones usando syscalls */
static void CursorDraw(int x, int y) UCODE;
static void CursorDraw(int x, int y)
{
    int row, col;
    for (row = 0; row < CURSOR_HEIGHT; row++) {
        for (col = 0; col < CURSOR_WIDTH; col++) {
            g_cursor_bg[row * CURSOR_WIDTH + col] = sys_get_pixel(x + col, y + row);
        }
    }
    g_cursor_saved_x = x;
    g_cursor_saved_y = y;
    for (row = 0; row < CURSOR_HEIGHT; row++) {
        for (col = 0; col < CURSOR_WIDTH; col++) {
            unsigned char pixel = cursor_bitmap[row][col];
            if      (pixel == 1) sys_draw_pixel(x + col, y + row, VGA_COLOR_WHITE);
            else if (pixel == 2) sys_draw_pixel(x + col, y + row, VGA_COLOR_BLACK);
        }
    }
}

static void CursorErase(int x, int y) UCODE;
static void CursorErase(int x, int y)
{
    int row, col;
    if (x < 0 || y < 0) return;
    for (row = 0; row < CURSOR_HEIGHT; row++) {
        for (col = 0; col < CURSOR_WIDTH; col++) {
            unsigned char bg = g_cursor_bg[row * CURSOR_WIDTH + col];
            sys_draw_pixel(x + col, y + row, bg);
        }
    }
}

/* funcion auxiliar, usada en la version completa del GUI */
static int GuiHitTest(int mx, int my, int x, int y, int w, int h) UCODE __attribute__((unused));
static int GuiHitTest(int mx, int my, int x, int y, int w, int h)
{
    return (mx >= x && mx < x+w && my >= y && my < y+h);
}

static void GuiDrawDesktop(void) UCODE;
static void GuiDrawDesktop(void)
{
    sys_fill_rect(0, 0, 640, 470, GUI_COLOR_DESKTOP);
    /* debug: after filling desktop, dump first portion of VRAM */
    __asm__ volatile (
        "mov %[num], %%eax\n"
        "int $0x30\n"
        : /* no outputs */
        : [num]"i"(SYS_DUMP_VRAM)
        : "%eax", "memory"
    );
    sys_draw_string(240, 210, USTR("Universidad de Guayaquil"), VGA_COLOR_WHITE, GUI_COLOR_DESKTOP);
    sys_draw_string(260, 222, USTR("System Operative Edit"), VGA_COLOR_LIGHT_GRAY, GUI_COLOR_DESKTOP);
}

static void GuiDrawHLine(int x, int y, int len, unsigned char color) UCODE;
static void GuiDrawHLine(int x, int y, int len, unsigned char color)
{
    sys_fill_rect(x, y, len, 1, color);
}

static void GuiDrawVLine(int x, int y, int len, unsigned char color) UCODE;
static void GuiDrawVLine(int x, int y, int len, unsigned char color)
{
    sys_fill_rect(x, y, 1, len, color);
}

static void GuiDrawButton(int x, int y, int w, int h, const char* label, int pressed) UCODE;
static void GuiDrawButton(int x, int y, int w, int h, const char* label, int pressed)
{
    sys_fill_rect(x, y, w, h, GUI_COLOR_BUTTON_BG);
    if (!pressed) {
        GuiDrawHLine(x,       y,       w,   VGA_COLOR_WHITE);
        GuiDrawVLine(x,       y,       h,   VGA_COLOR_WHITE);
        GuiDrawHLine(x,       y+h-1,   w,   GUI_COLOR_SHADOW);
        GuiDrawVLine(x+w-1,   y,       h,   GUI_COLOR_SHADOW);
    } else {
        GuiDrawHLine(x,       y,       w,   GUI_COLOR_SHADOW);
        GuiDrawVLine(x,       y,       h,   GUI_COLOR_SHADOW);
        GuiDrawHLine(x,       y+h-1,   w,   VGA_COLOR_WHITE);
        GuiDrawVLine(x+w-1,   y,       h,   VGA_COLOR_WHITE);
    }
    if (label) {
        /* calcula tx/ty sin strlen de libreria */
        int len = 0; const char* p = label; while (*p++) len++;
        int tx = x + (w - len * FONT_WIDTH) / 2;
        int ty = y + (h - FONT_HEIGHT) / 2;
        sys_draw_string(tx, ty, label, GUI_COLOR_BUTTON_TXT, GUI_COLOR_BUTTON_BG);
    }
}

static void GuiDrawWindow(WINDOW* win) UCODE;
static void GuiDrawWindow(WINDOW* win)
{
    if (!win || !win->visible) return;
    int x = win->x, y = win->y, w = win->w, h = win->h;
    sys_fill_rect(x+3, y+3, w, h, GUI_COLOR_SHADOW);
    sys_fill_rect(x, y, w, h, GUI_COLOR_WINDOW_BG);
    sys_fill_rect(x, y, w, TITLEBAR_HEIGHT, GUI_COLOR_TITLEBAR);
    if (win->title)
        sys_draw_string(x + 4, y + 1, win->title, GUI_COLOR_TITLEBAR_TXT, GUI_COLOR_TITLEBAR);
    GuiDrawButton(x + w - BUTTON_W - 2, y + 1, BUTTON_W, BUTTON_H, "X", 0);
    GuiDrawHLine(x,     y,     w,   VGA_COLOR_WHITE);
    GuiDrawVLine(x,     y,     h,   VGA_COLOR_WHITE);
    GuiDrawHLine(x,     y+h-1, w,   GUI_COLOR_SHADOW);
    GuiDrawVLine(x+w-1, y,     h,   GUI_COLOR_SHADOW);
    GuiDrawHLine(x, y + TITLEBAR_HEIGHT, w, GUI_COLOR_BORDER);
}

static void GuiDrawWindowText(WINDOW* win, int rx, int ry, const char* txt, unsigned char fg) UCODE;
static void GuiDrawWindowText(WINDOW* win, int rx, int ry, const char* txt, unsigned char fg)
{
    if (!win || !txt) return;
    sys_draw_string(win->x + BORDER_SIZE + rx,
                    win->y + TITLEBAR_HEIGHT + BORDER_SIZE + ry,
                    txt, fg, GUI_COLOR_WINDOW_BG);
}

static void GuiDrawTaskbar(void) UCODE;
static void GuiDrawTaskbar(void)
{
    int y = 470;
    sys_fill_rect(0, y, 640, 10, GUI_COLOR_BUTTON_BG);
    GuiDrawHLine(0, y, 640, VGA_COLOR_WHITE);
    GuiDrawButton(2, y + 1, 36, 8, USTR("Start"), 0);
    sys_draw_string(600, y + 1, USTR("00:00"), VGA_COLOR_BLACK, GUI_COLOR_BUTTON_BG);
}

/* punto de entrada del programa de usuario */
__attribute__((section(".user")))
void user_entry(void)
{
    /* simple debug: avisar al kernel que hemos llegado a Ring 3 */
    sys_debug(USTR("[user] entered Ring3\r\n"));


    SYS_MOUSE ms;
    int prev_x = 320, prev_y = 240;
    WINDOW welcome = { 150, 80, 340, 120, USTR("Bienvenido - System Operative Edit"), 1 };

    sys_debug(USTR("about to draw desktop\r\n"));
    GuiDrawDesktop();
    sys_debug(USTR("desktop done\r\n"));

    sys_debug(USTR("about to draw taskbar\r\n"));
    GuiDrawTaskbar();
    sys_debug(USTR("taskbar done\r\n"));

    sys_debug(USTR("about to draw welcome window\r\n"));
    GuiDrawWindow(&welcome);
    sys_debug(USTR("window done\r\n"));

    sys_debug(USTR("about to draw window text\r\n"));
    GuiDrawWindowText(&welcome, 10, 10, USTR("Sistema iniciado correctamente."), VGA_COLOR_BLACK);
    GuiDrawWindowText(&welcome, 10, 22, USTR("Universidad de Guayaquil v0.1"), VGA_COLOR_DARK_GRAY);
    sys_debug(USTR("window text done\r\n"));

    sys_debug(USTR("about to draw button\r\n"));
    GuiDrawButton(welcome.x + 130, welcome.y + 90, 60, 14, USTR("Aceptar"), 0);
    sys_debug(USTR("button done\r\n"));

    sys_debug(USTR("about to draw cursor\r\n"));
    CursorDraw(320, 240);
    sys_debug(USTR("cursor done\r\n"));

    while (1) {
        /* Leer estado del mouse por syscall */
        if (sys_get_mouse_state(&ms) == 0) {
            if (ms.x != prev_x || ms.y != prev_y) {
                CursorErase(prev_x, prev_y);
                CursorDraw(ms.x, ms.y);
                prev_x = ms.x;
                prev_y = ms.y;
            }
        }
        sys_yield();
    }
}

