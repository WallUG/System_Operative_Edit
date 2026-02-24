/*
 * ps2mouse.c - implementación del driver de ratón PS/2
 *
 * Contiene únicamente la lógica de protocolo PS/2 y el registro de posición.
 * No se encarga de dibujar el cursor; esa responsabilidad se delega al
 * subsistema de video (vga_cursor).
 */

#include "ps2mouse.h"

/* I/O port helpers ------------------------------------------------------- */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0,%1"::"a"(val),"Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t v;
    __asm__ volatile("inb %1,%0":"=a"(v):"Nd"(port));
    return v;
}

/* Esperar a que el buffer del PS/2 esté listo para escribir */
static void ps2_wait_write(void) {
    int timeout = 100000;
    while (timeout-- && (inb(0x64) & 0x02));
}

/* Esperar a que haya datos para leer */
static void ps2_wait_read(void) {
    int timeout = 100000;
    while (timeout-- && !(inb(0x64) & 0x01));
}

/* Enviar comando al ratón */
static void mouse_write(uint8_t cmd) {
    ps2_wait_write();
    outb(0x64, 0xD4);   /* indica que el siguiente byte va al mouse */
    ps2_wait_write();
    outb(0x60, cmd);
}

/* Leer byte del ratón */
static uint8_t mouse_read(void) {
    ps2_wait_read();
    return inb(0x60);
}

/* Estado global del ratón */
static MOUSE_STATE g_mouse = { 320, 240, 0, 1 };

void MouseInit(void) {
    uint8_t status;

    /* Desenmascarar IRQ1 (teclado/PS2) en el PIC master. */
    {
        uint8_t mask;
        __asm__ volatile("inb $0x21, %0" : "=a"(mask));
        mask &= ~0x02;   /* bit1 = IRQ1 = PS/2 */
        __asm__ volatile("outb %0, $0x21" :: "a"(mask));
    }

    /* Habilitar puerto auxiliar PS/2 (mouse) */
    ps2_wait_write();
    outb(0x64, 0xA8);

    /* Habilitar interrupciones del mouse en el controlador PS/2 */
    ps2_wait_write();
    outb(0x64, 0x20);
    ps2_wait_read();
    status = inb(0x60);
    status |= 0x02;            /* habilitar IRQ12 */
    ps2_wait_write();
    outb(0x64, 0x60);
    ps2_wait_write();
    outb(0x60, status);

    /* Resetear mouse */
    mouse_write(0xFF);
    mouse_read();  /* ACK */
    mouse_read();  /* 0xAA */
    mouse_read();  /* 0x00 */

    /* Habilitar streaming de datos */
    mouse_write(0xF4);
    mouse_read();  /* ACK */

    /* Posición inicial en el centro */
    g_mouse.x = 320;
    g_mouse.y = 240;
    g_mouse.buttons = 0;
    g_mouse.visible = 1;
}

void MouseRead(MOUSE_STATE* state) {
    /* Leer paquete de 3 bytes del mouse PS/2 si hay datos disponibles */
    if (!(inb(0x64) & 0x01)) return;

    uint8_t flags = inb(0x60);
    if (!(inb(0x64) & 0x01)) return;
    int8_t  dx    = (int8_t)inb(0x60);
    if (!(inb(0x64) & 0x01)) return;
    int8_t  dy    = (int8_t)inb(0x60);

    /* Aplicar movimiento (Y invertido en VGA) */
    g_mouse.x += dx;
    g_mouse.y -= dy;

    /* Clamp en pantalla */
    if (g_mouse.x < 0)   g_mouse.x = 0;
    if (g_mouse.x > 631) g_mouse.x = 631;
    if (g_mouse.y < 0)   g_mouse.y = 0;
    if (g_mouse.y > 468) g_mouse.y = 468;

    g_mouse.buttons = flags & 0x07;

    if (state) *state = g_mouse;
}

MOUSE_STATE* MouseGetState(void) {
    return &g_mouse;
}
