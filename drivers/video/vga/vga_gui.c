/*
 * vga_gui.c - Implementacion de la GUI tipo Windows
 */
#include <gui.h>          /* GUI_WINDOW, GUI_MOUSE_EVENT, prototypes */
/* minimal string helpers defined manually; no standard library available */

#include "vga_cursor.h"
#include "../../input/ps2mouse.h"
#include "vga_font.h"      /* VgaDrawString prototype */

/* tick counter defined in syscall.c (used by speaker/beep function) */
extern uint32_t get_tick_count(void);

/* ---------------------------------------------------------------------
   teclado + consola integrada
   --------------------------------------------------------------------- */

/* simple string routines used by console */
static int kg_strcmp(const char *a, const char *b) {
    while (*a && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}
static size_t kg_strlen(const char *s) {
    size_t i = 0;
    while (s[i]) i++;
    return i;
}
static void kg_strncpy(char *dst, const char *src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i]; i++) dst[i] = src[i];
    for (; i < n; i++) dst[i] = '\0';
}
static void kg_strncat(char *dst, const char *src, size_t max) {
    size_t len = kg_strlen(dst);
    size_t i = 0;
    while (len + i + 1 < max && src[i]) {
        dst[len + i] = src[i];
        i++;
    }
    if (len + i < max) dst[len + i] = '\0';
}

/* scancode -> character (set 1) mapping; ninguno de los caracteres
   requiere modificación con shift (se ignoran mayúsculas por simplicidad)
   F1 se convierte en código especial 0xF1 para alternar la consola. */
static const unsigned char scancode_to_ascii[128] = {
    [0x02] = '1', [0x03] = '2', [0x04] = '3', [0x05] = '4',
    [0x06] = '5', [0x07] = '6', [0x08] = '7', [0x09] = '8',
    [0x0A] = '9', [0x0B] = '0',
    [0x10] = 'q', [0x11] = 'w', [0x12] = 'e', [0x13] = 'r',
    [0x14] = 't', [0x15] = 'y', [0x16] = 'u', [0x17] = 'i',
    [0x18] = 'o', [0x19] = 'p',
    [0x1E] = 'a', [0x1F] = 's', [0x20] = 'd', [0x21] = 'f',
    [0x22] = 'g', [0x23] = 'h', [0x24] = 'j', [0x25] = 'k',
    [0x26] = 'l',
    [0x2C] = 'z', [0x2D] = 'x', [0x2E] = 'c', [0x2F] = 'v',
    [0x30] = 'b', [0x31] = 'n', [0x32] = 'm',
    [0x39] = ' ', /* espacio */
    [0x1C] = '\n', /* enter */
    [0x0E] = 0x08, /* backspace */
    [0x3B] = 0xF1, /* F1 toggle */
};

#define KBD_BUF_SIZE 128
static char kbd_buf[KBD_BUF_SIZE];
static int kbd_head = 0, kbd_tail = 0;

static void kbd_put(char c)
{
    int next = (kbd_head + 1) % KBD_BUF_SIZE;
    if (next == kbd_tail) {
        /* buffer lleno, descartar */
        return;
    }
    kbd_buf[kbd_head] = c;
    kbd_head = next;
}

/* retorna -1 si no hay caracteres */
static int kbd_getchar(void)
{
    if (kbd_head == kbd_tail) return -1;
    int c = (unsigned char)kbd_buf[kbd_tail];
    kbd_tail = (kbd_tail + 1) % KBD_BUF_SIZE;
    return c;
}

/* Invocado desde el manejador de IRQ1 en idt.c */
void GuiKeyboardHandler(uint8_t sc)
{
    /* ignorar releases (bit7 set) */
    if (sc & 0x80) return;
    if (sc < sizeof(scancode_to_ascii) && scancode_to_ascii[sc]) {
        kbd_put(scancode_to_ascii[sc]);
    }
}

/* ---------------------------------------------------------------------
   consola gráfica simple
   --------------------------------------------------------------------- */

#define CONS_COLS 70
#define CONS_ROWS 12
static char cons_history[CONS_ROWS][CONS_COLS+1];
static int cons_history_start = 0;
static int cons_history_count = 0;
static char cons_input[CONS_COLS+1];
static int cons_input_pos = 0;
static int console_active = 0;

static GUI_WINDOW console_win = { 10, 10, 620, 200, "Console", 0 };

static void ConsoleRedraw(void);

static void ConsoleClear(void)
{
    for (int i = 0; i < CONS_ROWS; i++)
        cons_history[i][0] = '\0';
    cons_history_start = cons_history_count = 0;
    cons_input_pos = 0;
    cons_input[0] = '\0';
    ConsoleRedraw();
}

static void ConsoleAddLine(const char *line)
{
    int idx;
    if (cons_history_count < CONS_ROWS) {
        idx = (cons_history_start + cons_history_count) % CONS_ROWS;
        cons_history_count++;
    } else {
        /* desplazar (sobrescribir la línea más antigua) */
        idx = cons_history_start;
        cons_history_start = (cons_history_start + 1) % CONS_ROWS;
    }
    kg_strncpy(cons_history[idx], line, CONS_COLS);
    cons_history[idx][CONS_COLS] = '\0';
    ConsoleRedraw();
}

static void ConsolePrint(const char *s)
{
    /* imprime cadena con saltos de línea automáticamente */
    const char *p = s;
    char buf[CONS_COLS+1];
    int bi = 0;
    while (*p) {
        if (*p == '\n') {
            buf[bi] = '\0';
            ConsoleAddLine(buf);
            bi = 0;
        } else {
            if (bi < CONS_COLS - 1)
                buf[bi++] = *p;
        }
        p++;
    }
    if (bi > 0) {
        buf[bi] = '\0';
        ConsoleAddLine(buf);
    }
}

static void console_execute(const char *cmd)
{
    if (kg_strcmp(cmd, "help") == 0) {
        ConsolePrint("help - lista de comandos\n");
        ConsolePrint("clear - limpiar pantalla\n");
    } else if (kg_strcmp(cmd, "clear") == 0) {
        ConsoleClear();
    } else {
        char buf[CONS_COLS+1];
        kg_strncpy(buf, "comando desconocido: ", CONS_COLS);
        kg_strncat(buf, cmd, CONS_COLS + 1);
        ConsoleAddLine(buf);
    }
}

static void ConsoleProcessChar(int ch)
{
    if (ch == 0xF1) {
        /* toggle */
        console_active = !console_active;
        console_win.visible = console_active;
        if (console_active) {
            ConsoleClear();
            GuiDrawWindow(&console_win);
            ConsolePrint("Consola activa. Escribe 'help' para ayuda.\n");
        } else {
            /* al cerrar, redibujar desktop/taskbar para limpiar */
            GuiDrawDesktop();
            GuiDrawTaskbar();
        }
        return;
    }
    if (!console_active) return;

    if (ch == '\n') {
        cons_input[cons_input_pos] = '\0';
        ConsoleAddLine(cons_input);
        console_execute(cons_input);
        cons_input_pos = 0;
        cons_input[0] = '\0';
    } else if (ch == 0x08) {
        if (cons_input_pos > 0) {
            cons_input_pos--;
            cons_input[cons_input_pos] = '\0';
            ConsoleRedraw();
        }
    } else {
        if (cons_input_pos < CONS_COLS - 1) {
            cons_input[cons_input_pos++] = (char)ch;
            cons_input[cons_input_pos] = '\0';
            ConsoleRedraw();
        }
    }
}

static void ConsoleProcess(void)
{
    int c;
    while ((c = kbd_getchar()) != -1) {
        ConsoleProcessChar(c);
    }
}

static void ConsoleRedraw(void)
{
    /* repinta toda la ventana cuando cambia el contenido */
    if (!console_active) return;
    GuiDrawWindow(&console_win);
    /* historial */
    for (int i = 0; i < cons_history_count; i++) {
        int idx = (cons_history_start + i) % CONS_ROWS;
        int y = console_win.y + TITLEBAR_HEIGHT + BORDER_SIZE + i * FONT_HEIGHT;
        VgaDrawString(console_win.x + BORDER_SIZE, y, cons_history[idx], VGA_COLOR_BLACK, GUI_COLOR_WINDOW_BG);
    }
    /* línea de entrada actual */
    int y = console_win.y + TITLEBAR_HEIGHT + BORDER_SIZE + cons_history_count * FONT_HEIGHT;
    /* borra área de entrada */
    VgaFillRect(console_win.x + BORDER_SIZE, y, CONS_COLS * FONT_WIDTH, FONT_HEIGHT, GUI_COLOR_WINDOW_BG);
    if (cons_input_pos > 0) {
        VgaDrawString(console_win.x + BORDER_SIZE, y, cons_input, VGA_COLOR_BLACK, GUI_COLOR_WINDOW_BG);
    }
}

/* ---------------------------------------------------------------------
   sonido de PC speaker
   --------------------------------------------------------------------- */

/* forward declarations for port I/O helpers (defined later) */
static inline void outb(uint16_t port, uint8_t val);
static inline uint8_t inb(uint16_t port);


static void pc_speaker_beep(unsigned freq, unsigned ticks)
{
    /* configurar PIT canal 2 */
    unsigned divisor = 1193180 / freq;
    outb(0x43, 0xB6);
    outb(0x42, divisor & 0xFF);
    outb(0x42, divisor >> 8);
    /* activar speaker */
    uint8_t tmp = inb(0x61);
    if ((tmp & 3) != 3) outb(0x61, tmp | 3);
    /* esperar 'ticks' unidades usando contador de ticks global */
    uint32_t start = get_tick_count();
    while ((get_tick_count() - start) < ticks) {
        /* busy-wait */
        __asm__ volatile("hlt");
    }
    /* desactivar speaker */
    tmp = inb(0x61) & ~3;
    outb(0x61, tmp);
}

static void PlayStartupSound(void)
{
    /* tono ascendente breve */
    pc_speaker_beep(440, 25);
    pc_speaker_beep(880, 25);
}

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
    /* keyboard buffer */
    kbd_head = kbd_tail = 0;
    /* limpiar consola historia */
    ConsoleClear();
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

    /* sonar el timbre de inicio y dibujar cursor inicial */
    PlayStartupSound();
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
        }

        /* procesar teclado para consola si está abierto o si se presionó F1 */
        ConsoleProcess();

        prev_buttons = ms->buttons;
    }
}
