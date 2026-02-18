/*
 * vga_mouse.c - Implementacion del mouse PS/2 con cursor grafico
 */
#include "vga_mouse.h"
#include "vga_font.h"
#include "vga.h"   /* VgaGetPixel - shadow framebuffer */

/* I/O helpers */
static inline void outb_m(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0,%1"::"a"(val),"Nd"(port));
}
static inline uint8_t inb_m(uint16_t port) {
    uint8_t v;
    __asm__ volatile("inb %1,%0":"=a"(v):"Nd"(port));
    return v;
}

/* Esperar a que el buffer del PS/2 este listo para escribir */
static void ps2_wait_write(void) {
    int timeout = 100000;
    while (timeout-- && (inb_m(0x64) & 0x02));
}

/* Esperar a que haya datos para leer */
static void ps2_wait_read(void) {
    int timeout = 100000;
    while (timeout-- && !(inb_m(0x64) & 0x01));
}

/* Enviar comando al mouse */
static void mouse_write(uint8_t cmd) {
    ps2_wait_write();
    outb_m(0x64, 0xD4);   /* indica que el siguiente byte va al mouse */
    ps2_wait_write();
    outb_m(0x60, cmd);
}

/* Leer byte del mouse */
static uint8_t mouse_read(void) {
    ps2_wait_read();
    return inb_m(0x60);
}

/* Estado global del mouse */
static MOUSE_STATE g_mouse = { 320, 240, 0, 1 };

/* Buffer del fondo detras del cursor - guardamos el color de cada pixel
 * para restaurarlo al mover. Inicializamos a negro (color del desktop). */
static UCHAR g_cursor_bg[MOUSE_WIDTH * MOUSE_HEIGHT];  /* fondo guardado */
static INT   g_cursor_saved_x = -1;  /* posicion donde guardamos el fondo */
static INT   g_cursor_saved_y = -1;

/* Bitmap del cursor (flecha) - 1=pixel visible, 0=transparente */
static const unsigned char cursor_bitmap[MOUSE_HEIGHT][MOUSE_WIDTH] = {
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
/* 1=blanco, 2=negro (borde), 0=transparente */

VOID MouseInit(VOID)
{
    uint8_t status;

    /* Desenmascarar IRQ1 (teclado/PS2) en el PIC master ahora que el
     * driver esta listo para recibirla. IRQ1 = bit1 del registro de mascaras.
     * Leemos la mascara actual, limpiamos bit1 y escribimos de vuelta. */
    {
        /* Puerto 0x21 = PIC1 data/mask */
        uint8_t mask;
        __asm__ volatile("inb $0x21, %0" : "=a"(mask));
        mask &= ~0x02;   /* bit1 = IRQ1 = PS/2 teclado/mouse */
        __asm__ volatile("outb %0, $0x21" :: "a"(mask));
    }

    /* Habilitar puerto auxiliar PS/2 (mouse) */
    ps2_wait_write();
    outb_m(0x64, 0xA8);

    /* Habilitar interrupciones del mouse en el controlador PS/2 */
    ps2_wait_write();
    outb_m(0x64, 0x20);        /* leer byte de configuracion */
    ps2_wait_read();
    status = inb_m(0x60);
    status |= 0x02;            /* habilitar IRQ12 */
    ps2_wait_write();
    outb_m(0x64, 0x60);        /* escribir byte de configuracion */
    ps2_wait_write();
    outb_m(0x60, status);

    /* Resetear mouse */
    mouse_write(0xFF);
    mouse_read();  /* ACK */
    mouse_read();  /* 0xAA (self-test ok) */
    mouse_read();  /* 0x00 (device ID) */

    /* Habilitar streaming de datos */
    mouse_write(0xF4);
    mouse_read();  /* ACK */

    /* Posicion inicial en el centro */
    g_mouse.x = 320;
    g_mouse.y = 240;
    g_mouse.buttons = 0;
    g_mouse.visible = 1;
}

VOID MouseRead(MOUSE_STATE* state)
{
    /* Leer paquete de 3 bytes del mouse PS/2 si hay datos disponibles */
    if (!(inb_m(0x64) & 0x01)) return;  /* no hay datos */

    uint8_t flags = inb_m(0x60);
    if (!(inb_m(0x64) & 0x01)) return;
    int8_t  dx    = (int8_t)inb_m(0x60);
    if (!(inb_m(0x64) & 0x01)) return;
    int8_t  dy    = (int8_t)inb_m(0x60);

    /* Aplicar movimiento (Y invertido en VGA) */
    g_mouse.x += dx;
    g_mouse.y -= dy;

    /* Clamp dentro de la pantalla */
    if (g_mouse.x < 0)   g_mouse.x = 0;
    if (g_mouse.x > 631) g_mouse.x = 631;
    if (g_mouse.y < 0)   g_mouse.y = 0;
    if (g_mouse.y > 468) g_mouse.y = 468;

    /* Botones: bit0=izq, bit1=der, bit2=medio */
    g_mouse.buttons = flags & 0x07;

    if (state) *state = g_mouse;
}

VOID MouseDraw(INT x, INT y)
{
    int row, col;

    /* Guardar el fondo REAL usando el shadow framebuffer.
     * VgaGetPixel() lee de la copia en RAM (sin tocar el hardware VGA),
     * por eso es exacto independientemente del color que haya debajo. */
    for (row = 0; row < MOUSE_HEIGHT; row++) {
        for (col = 0; col < MOUSE_WIDTH; col++) {
            g_cursor_bg[row * MOUSE_WIDTH + col] = VgaGetPixel(x + col, y + row);
        }
    }
    g_cursor_saved_x = x;
    g_cursor_saved_y = y;

    /* Dibujar cursor encima del fondo guardado */
    for (row = 0; row < MOUSE_HEIGHT; row++) {
        for (col = 0; col < MOUSE_WIDTH; col++) {
            unsigned char pixel = cursor_bitmap[row][col];
            if      (pixel == 1) VgaPutPixel(x + col, y + row, VGA_COLOR_WHITE);
            else if (pixel == 2) VgaPutPixel(x + col, y + row, VGA_COLOR_BLACK);
        }
    }
}

VOID MouseErase(INT x, INT y)
{
    int row, col;
    if (x < 0 || y < 0) return;

    /* Restaurar TODOS los pixels del area del cursor desde el fondo guardado.
     * Restauramos incluso los transparentes (pixel==0) porque el cursor
     * pudo haber corrompido bytes compartidos del framebuffer planar. */
    for (row = 0; row < MOUSE_HEIGHT; row++) {
        for (col = 0; col < MOUSE_WIDTH; col++) {
            UCHAR bg = g_cursor_bg[row * MOUSE_WIDTH + col];
            VgaPutPixel(x + col, y + row, bg);
        }
    }
}

MOUSE_STATE* MouseGetState(VOID) {
    return &g_mouse;
}
