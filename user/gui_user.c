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
    int prev_x = 320, prev_y = 240;
    GUI_WINDOW welcome;
    /* inicializamos campo por campo para evitar advertencias de inicializador */
    welcome.x = 150;
    welcome.y = 80;
    welcome.w = 340;
    welcome.h = 120;
    welcome.title = USTR("Bienvenido - System Operative Edit");
    welcome.visible = 1;

    /* dibujar todo usando el servicio GUI del kernel */
    sys_debug(USTR("about to draw desktop\r\n"));
    sys_gui_draw_desktop();
    sys_debug(USTR("desktop done\r\n"));

    sys_debug(USTR("about to draw taskbar\r\n"));
    sys_gui_draw_taskbar();
    sys_debug(USTR("taskbar done\r\n"));

    sys_debug(USTR("about to draw welcome window\r\n"));
    sys_gui_draw_window(&welcome);
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
            sys_gui_draw_taskbar();
            last_secs = secs;
        }

        /* procesar eventos de mouse */
        if (sys_get_mouse_event(&ms) == 0) {
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

