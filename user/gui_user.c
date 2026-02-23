#include <libsys.h>
#include <types.h>
#include <video/vga/vga.h>    /* colores VGA y prototipos de VGA driver */
#include <video/vga/vga_font.h> /* VgaDrawString prototype and font constants */
#include <gui.h>   /* estructuras y colores GUI */
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


/* punto de entrada del programa de usuario */
__attribute__((section(".user")))
void user_entry(void)
{
    /* simple debug: avisar al kernel que hemos llegado a Ring 3 */
    sys_debug(USTR("[user] entered Ring3\r\n"));


    SYS_MOUSE ms;
    /* cursor se dibuja desde kernel, no necesitamos prev_x/prev_y aquí */
    (void)ms;
    /* ventana de bienvenida en memoria de usuario (sección .user.data) */
    static const char welcome_title[] URODATA = "Bienvenido - System Operative Edit";
    static GUI_WINDOW welcome UDATA = {
        .x = 150,
        .y = 80,
        .w = 340,
        .h = 120,
        .title = welcome_title,
        .visible = 1
    };

    /* dibujar todo usando el servicio GUI del kernel */
    sys_debug(USTR("about to draw desktop\r\n"));
    sys_gui_draw_desktop();
    sys_debug(USTR("desktop done\r\n"));

    sys_debug(USTR("about to draw taskbar\r\n"));
    sys_gui_draw_taskbar();
    sys_debug(USTR("taskbar done\r\n"));

    /* report address of welcome struct for debugging */
    {
        uint32_t ptr = (uint32_t)&welcome;
        char hex[9];
        const char *h="0123456789ABCDEF";
        for (int i = 0; i < 8; i++) {
            hex[7 - i] = h[(ptr >> (i * 4)) & 0xF];
        }
        hex[8] = '\0';
        sys_debug(USTR("welcome addr=0x"));
        sys_debug(hex);
        sys_debug(USTR("\r\n"));
    }
    sys_debug(USTR("about to draw welcome window\r\n"));
    sys_gui_draw_window(welcome.x, welcome.y, welcome.w, welcome.h, welcome_title);
    sys_debug(USTR("window done\r\n"));

    sys_debug(USTR("about to draw window text\r\n"));
    sys_gui_draw_window_text(&welcome, 10, 10, USTR("Sistema iniciado correctamente."), VGA_COLOR_BLACK);
    sys_gui_draw_window_text(&welcome, 10, 22, USTR("Universidad de Guayaquil v0.1"), VGA_COLOR_DARK_GRAY);
    sys_debug(USTR("window text done\r\n"));

    /* ya no manejamos cursor manualmente, el kernel se ocupa */

    uint32_t last_secs = 0;
    int start_pressed = 0;
    while (1) {
        /* actualizar reloj cada segundo */
        uint32_t ticks = sys_get_tick();
        uint32_t secs = ticks / 100;
        if (secs != last_secs) {
            /* debug: mostrar número de ticks y segundos obtenidos */
            {
                char buf[32];
                int n = 0;
                uint32_t tmp = ticks;
                /* simple decimal conversion */
                if (tmp == 0) buf[n++] = '0';
                else {
                    char dec[12]; int di = 0;
                    while (tmp) { dec[di++] = '0' + (tmp % 10); tmp /= 10; }
                    while (di--) buf[n++] = dec[di];
                }
                buf[n++] = ' '; buf[n++] = 't'; buf[n++] = 'i'; buf[n++] = 'c'; buf[n++] = 'k';
                buf[n++] = '\r'; buf[n++] = '\n'; buf[n] = '\0';
                sys_debug(buf);
            }
            sys_gui_draw_taskbar();
            last_secs = secs;
        }

        /* procesar eventos de mouse */
        if (sys_get_mouse_event(&ms) == 0) {
            /* debug: log mouse event */
            {
                /* convertir números a decimal manualmente */
                char buf[64];
                int n = 0;
                buf[n++] = '['; buf[n++] = 'u'; buf[n++] = 's'; buf[n++] = 'e'; buf[n++] = 'r'; buf[n++] = ' ';
                buf[n++] = 'e'; buf[n++] = 'v'; buf[n++] = 'e'; buf[n++] = 'n'; buf[n++] = 't'; buf[n++] = ']'; buf[n++] = ' ';
                /* x value */
                int v = ms.x;
                if (v < 0) { buf[n++] = '-'; v = -v; }
                {
                    char dec[12]; int di = 0;
                    if (v == 0) dec[di++] = '0';
                    while (v) { dec[di++] = '0' + (v % 10); v /= 10; }
                    while (di--) buf[n++] = dec[di];
                }
                buf[n++] = ' ';
                buf[n++] = 'y'; buf[n++] = '=';
                v = ms.y;
                if (v < 0) { buf[n++] = '-'; v = -v; }
                {
                    char dec[12]; int di = 0;
                    if (v == 0) dec[di++] = '0';
                    while (v) { dec[di++] = '0' + (v % 10); v /= 10; }
                    while (di--) buf[n++] = dec[di];
                }
                buf[n++] = ' ';
                buf[n++] = 'b'; buf[n++] = 't'; buf[n++] = 'n'; buf[n++] = '=';
                v = ms.buttons;
                if (v < 0) { buf[n++] = '-'; v = -v; }
                {
                    char dec[12]; int di = 0;
                    if (v == 0) dec[di++] = '0';
                    while (v) { dec[di++] = '0' + (v % 10); v /= 10; }
                    while (di--) buf[n++] = dec[di];
                }
                buf[n++] = '\r'; buf[n++] = '\n';
                buf[n] = '\0';
                sys_debug(buf);
            }
            /* detectar clic en "Start" */
            if ((ms.buttons & 1) && !start_pressed &&
                ms.x >= 2 && ms.x < 38 && ms.y >= 471 && ms.y < 471+8) {
                start_pressed = 1;
                sys_gui_draw_taskbar();
            } else if (!(ms.buttons & 1) && start_pressed) {
                start_pressed = 0;
                sys_gui_draw_taskbar();
            }
        }

        sys_yield();
    }
}

