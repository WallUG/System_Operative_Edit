/*
 * vga_gui.c - Implementacion de la GUI tipo Windows
 */
#include <gui.h>          /* GUI_WINDOW, GUI_MOUSE_EVENT, prototypes */
#include "vga_cursor.h"
#include "../../input/ps2mouse.h"
#include "vga_font.h"      /* VgaDrawString prototype */

/* serial output available from kernel for debugging */
extern void serial_puts(const char *s);
extern void serial_print_hex(uint32_t v);

/* basic port io helpers (copied from ps2mouse) */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0,%1"::"a"(val),"Nd"(port));
}
static inline uint8_t inb(uint16_t port) {
    uint8_t v;
    __asm__ volatile("inb %1,%0":"=a"(v):"Nd"(port));
    return v;
}

/* tick counter defined in syscall.c */
extern uint32_t get_tick_count(void);

/* Leer hora del RTC CMOS (BCD) */
static uint8_t bcd2bin(uint8_t v) { return (v & 0x0F) + ((v >> 4) * 10); }
static void read_rtc(uint8_t *h, uint8_t *m, uint8_t *s)
{
    /* deshabilitar interrupts para acceder CMOS de forma atomica */
    __asm__ volatile("cli");
    outb(0x70, 0x00);
    *s = bcd2bin(inb(0x71));
    outb(0x70, 0x02);
    *m = bcd2bin(inb(0x71));
    outb(0x70, 0x04);
    *h = bcd2bin(inb(0x71));
    __asm__ volatile("sti");
}

/* nota: el driver de entrada (ps2mouse) define MouseInit/MouseRead/MouseGetState */

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

void GuiDrawWindow(const GUI_WINDOW* win)
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

void GuiDrawWindowText(const GUI_WINDOW* win, INT rx, INT ry, const char* txt, UCHAR fg)
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


/* evento de mouse almacenado en cola circular */
#define MOUSE_QUEUE_SIZE 32
static GUI_MOUSE_EVENT g_mouse_queue[MOUSE_QUEUE_SIZE];
static volatile int g_mouse_head = 0, g_mouse_tail = 0;

/* Inicializar subsistema GUI (cursor + cola de eventos) */
void GuiInit(void)
{
    g_mouse_head = g_mouse_tail = 0;
    CursorInit();
}

/* insertar evento en cola; descartar si llena */
void GuiQueueMouseEvent(int x, int y, int buttons)
{
    int next = (g_mouse_head + 1) % MOUSE_QUEUE_SIZE;
    if (next == g_mouse_tail) {
        /* cola llena, descartar event */
        return;
    }
    g_mouse_queue[g_mouse_head].x = x;
    g_mouse_queue[g_mouse_head].y = y;
    g_mouse_queue[g_mouse_head].buttons = buttons;
    g_mouse_head = next;
}

/* extraer evento; retorna 0 éxito, -1 vacía */
int GuiGetMouseEvent(GUI_MOUSE_EVENT* out)
{
    if (g_mouse_head == g_mouse_tail) return -1;
    *out = g_mouse_queue[g_mouse_tail];
    g_mouse_tail = (g_mouse_tail + 1) % MOUSE_QUEUE_SIZE;
    return 0;
}

VOID GuiMainLoop(VOID)
{
    MOUSE_STATE* ms;
    INT prev_x = 320, prev_y = 240;
    int prev_buttons = 0;

    /* inicializaciones comunes */
    GuiInit();

    /* dibujar cursor inicial (respeta bandera visible en ps2mouse) */
    CursorDraw(prev_x, prev_y);

    /* mostrar hora BIOS en taskbar al inicio + fecha */
    {
        uint8_t h,m,s,d,mo,y;
        read_rtc(&h,&m,&s);
        /* read extra fields day/month/year */
        __asm__ volatile("cli");
        outb(0x70, 0x07); d = bcd2bin(inb(0x71));
        outb(0x70, 0x08); mo = bcd2bin(inb(0x71));
        outb(0x70, 0x09); y = bcd2bin(inb(0x71));
        __asm__ volatile("sti");
        char clock[32];
        int n = 0;
        /* manual zero-padded conversion */
        clock[n++] = '0' + (h/10); clock[n++] = '0' + (h%10);
        clock[n++] = ':';
        clock[n++] = '0' + (m/10); clock[n++] = '0' + (m%10);
        clock[n++] = ':';
        clock[n++] = '0' + (s/10); clock[n++] = '0' + (s%10);
        clock[n++] = ' ';
        clock[n++] = '0' + (d/10); clock[n++] = '0' + (d%10);
        clock[n++] = '/';
        clock[n++] = '0' + (mo/10); clock[n++] = '0' + (mo%10);
        clock[n++] = '/';
        clock[n++] = '0' + (y/10); clock[n++] = '0' + (y%10);
        clock[n] = '\0';
        VgaDrawString(600, 471+1, clock, VGA_COLOR_BLACK, GUI_COLOR_BUTTON_BG);
    }

    /* reloj interno: refrescar desde CMOS cada tick */
    uint32_t last_tick = get_tick_count();
    int menu_open = 0;
    /* trail buffer for fade-out effect (desktop only) */
    #define TRAIL_MAX 8
    #define TRAIL_LIFE 5
    struct {int x,y,life;} trail[TRAIL_MAX];
    for (int __i = 0; __i < TRAIL_MAX; __i++) {
        trail[__i].life = 0;
    }

    while (1) {
        __asm__ volatile("sti; hlt");

        /* actualizar reloj y fecha cada tick leyendo la RTC */
        {
            uint32_t ticks = get_tick_count();
            if (ticks != last_tick) {
                last_tick = ticks;
                uint8_t h,m,s,d,mo,y;
                read_rtc(&h,&m,&s);
                /* also read date fields */
                __asm__ volatile("cli");
                outb(0x70, 0x07); d = bcd2bin(inb(0x71));
                outb(0x70, 0x08); mo = bcd2bin(inb(0x71));
                outb(0x70, 0x09); y = bcd2bin(inb(0x71));
                __asm__ volatile("sti");
                char clock[24];
                int n = 0;
                /* hh:mm:ss dd/mm/yy */
                clock[n++] = '0' + (h/10); clock[n++] = '0' + (h%10);
                clock[n++] = ':';
                clock[n++] = '0' + (m/10); clock[n++] = '0' + (m%10);
                clock[n++] = ':';
                clock[n++] = '0' + (s/10); clock[n++] = '0' + (s%10);
                clock[n++] = ' ';
                clock[n++] = '0' + (d/10); clock[n++] = '0' + (d%10);
                clock[n++] = '/';
                clock[n++] = '0' + (mo/10); clock[n++] = '0' + (mo%10);
                clock[n++] = '/';
                clock[n++] = '0' + (y/10); clock[n++] = '0' + (y%10);
                clock[n] = '\0';
                /* if cursor sits on taskbar, erase it NOW so we can
                   redraw after the clock update and refresh the buffer */
                if (prev_y >= 470) {
                    CursorErase(prev_x, prev_y);
                }

                /* erase entire previous string area and draw new time */
                VgaFillRect(600, 471+1, 16*8, 8, GUI_COLOR_BUTTON_BG);
                VgaDrawString(600, 471+1, clock, VGA_COLOR_BLACK, GUI_COLOR_BUTTON_BG);

                /* update trail fading too (desktop only) */
                for (int ti = 0; ti < TRAIL_MAX; ti++) {
                    if (trail[ti].life > 0) {
                        trail[ti].life--;
                        if (trail[ti].life > 0) {
                            /* skip taskbar area */
                            if (trail[ti].y < 470) {
                                UCHAR col = (trail[ti].life >= 3) ? VGA_COLOR_LIGHT_GRAY : VGA_COLOR_DARK_GRAY;
                                VgaPutPixel(trail[ti].x, trail[ti].y, col);
                            }
                        } else {
                            if (trail[ti].y < 470) {
                                /* erase to desktop color */
                                VgaPutPixel(trail[ti].x, trail[ti].y, GUI_COLOR_DESKTOP);
                            }
                        }
                    }
                }

                /* animate cursor on tick rather than movement */
                CursorToggleInvert();
                if (prev_y >= 470) {
                    CursorDraw(prev_x, prev_y);
                }
            }
        }

        /* procesar datos PS/2 solo si hay */
        ms = MouseGetState();
        MouseRead(ms);

        /* encolar evento cuando cambie posicion o botones */
        if (ms->x != prev_x || ms->y != prev_y || ms->buttons != prev_buttons) {
            /* push old location onto trail buffer */
            for (int ti = TRAIL_MAX-1; ti > 0; ti--) {
                trail[ti] = trail[ti-1];
            }
            trail[0].x = prev_x;
            trail[0].y = prev_y;
            trail[0].life = TRAIL_LIFE;

            GuiQueueMouseEvent(ms->x, ms->y, ms->buttons);
            /* simple cursor animation by toggling invert flag */
            CursorToggleInvert();
            CursorErase(prev_x, prev_y);
            CursorDraw(ms->x, ms->y);
            prev_x = ms->x;
            prev_y = ms->y;
            /* handle button changes */
            if (ms->buttons != prev_buttons) {
                /* left click toggles start menu when clicked on button */
                if ((ms->buttons & 1) && prev_buttons == 0) {
                    if (prev_x >= 2 && prev_x < 38 && prev_y >= 471 && prev_y < 471+8) {
                        menu_open = !menu_open;
                        if (menu_open) {
                            VgaFillRect(2, 460, 100, 40, VGA_COLOR_LIGHT_GRAY);
                            VgaDrawString(4, 462, "Menu Item", VGA_COLOR_BLACK, VGA_COLOR_LIGHT_GRAY);
                        } else {
                            /* redraw taskbar to erase any artefacts */
                            GuiDrawTaskbar();
                        }
                    } else {
                        /* if menu open and click inside menu area, treat as item selection */
                        if (menu_open && prev_x >= 2 && prev_x < 102 && prev_y >= 460 && prev_y < 500) {
                            serial_puts("[gui] menu item clicked\r\n");
                            /* close menu after selection */
                            GuiDrawTaskbar();
                            menu_open = 0;
                        } else if (menu_open) {
                            /* click outside menu closes it */
                            GuiDrawTaskbar();
                            menu_open = 0;
                        }
                    }
                }
                /* right click debug output */
                if ((ms->buttons & 2) && !(prev_buttons & 2)) {
                    /* print coordinates to serial console */
                    serial_puts("[gui] right click at ");
                    serial_print_hex(prev_x);
                    serial_puts(",");
                    serial_print_hex(prev_y);
                    serial_puts("\r\n");
                }
            }
            prev_buttons = ms->buttons;
        }
    }
}
