/*
 * boot_splash.c -- Animacion de arranque del kernel (modo VGA 13h nativo)
 *
 * --- POR QUE ESTE ARCHIVO ES NECESARIO ---
 *
 * La cadena de boot REAL es:
 *   GRUB --> multiboot /boot/kernel.elf --> boot.asm::start --> kernel_main()
 *
 * El directorio boot/freeldr/ compila a un binario separado (freeldr.sys)
 * que NUNCA entra en esa cadena. La animacion que tiene alli dentro
 * es codigo muerto; nadie la ejecuta.
 *
 * Este modulo vive dentro del propio kernel.elf y es llamado como
 * primera linea de kernel_main(), antes de GDT/IDT/screen.
 *
 * --- POR QUE NO USAMOS INT 0x10 DEL BIOS ---
 *
 * GRUB deja la CPU en modo protegido 32-bit. INT 0x10 y INT 0x15 del
 * BIOS requieren modo real o v8086 -- causarian un GPF / triple-fault en
 * hardware real (en QEMU funciona por azar porque QEMU intercepta la int).
 *
 * En su lugar escribimos DIRECTAMENTE en los registros VGA por puerto I/O.
 * Esto funciona en modo protegido, en hardware real, en QEMU y en VirtualBox.
 *
 * --- SECUENCIA ---
 *   1. Programar VGA para modo 13h (320x200, 256 colores) por puertos
 *   2. Configurar paleta institucional UG en el DAC
 *   3. Dibujar logo UG + barra de progreso (5 fases, ~2.5 s)
 *   4. Restaurar modo texto 80x25 (modo 3) por puertos
 *
 * LICENCIA: GPL-3.0
 */

#include "boot_splash.h"

/* ===========================================================================
 * SECCION 1: ACCESO A PUERTOS I/O
 * =========================================================================== */

static inline void outb(unsigned short port, unsigned char val)
{
    __asm__ volatile("outb %0, %1" :: "a"(val), "Nd"(port));
}

static inline unsigned char inb(unsigned short port)
{
    unsigned char val;
    __asm__ volatile("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

/* ===========================================================================
 * SECCION 2: REGISTROS VGA
 *
 * Referencia: FreeVGA Project -- http://www.osdever.net/FreeVGA/home.htm
 * =========================================================================== */

#define VGA_MISC_WRITE   0x3C2
#define VGA_SEQ_INDEX    0x3C4
#define VGA_SEQ_DATA     0x3C5
#define VGA_DAC_WRITE    0x3C8
#define VGA_DAC_DATA     0x3C9
#define VGA_DAC_READ     0x3C7
#define VGA_GC_INDEX     0x3CE
#define VGA_GC_DATA      0x3CF
#define VGA_AC_INDEX     0x3C0
#define VGA_AC_WRITE     0x3C0
#define VGA_INSTAT_READ  0x3DA
#define VGA_CRTC_INDEX   0x3D4
#define VGA_CRTC_DATA    0x3D5

#define VGA13_FB   ((unsigned char*)0xA0000)
#define VGA13_W    320
#define VGA13_H    200

#define TEXTFB     ((unsigned short*)0xB8000)
#define TEXT_COLS  80
#define TEXT_ROWS  25

static inline void seq_w(unsigned char idx, unsigned char val)
    { outb(VGA_SEQ_INDEX, idx); outb(VGA_SEQ_DATA, val); }

static inline void crtc_w(unsigned char idx, unsigned char val)
    { outb(VGA_CRTC_INDEX, idx); outb(VGA_CRTC_DATA, val); }

static inline unsigned char crtc_r(unsigned char idx)
    { outb(VGA_CRTC_INDEX, idx); return inb(VGA_CRTC_DATA); }

static inline void gc_w(unsigned char idx, unsigned char val)
    { outb(VGA_GC_INDEX, idx); outb(VGA_GC_DATA, val); }

static inline void ac_w(unsigned char idx, unsigned char val)
    { inb(VGA_INSTAT_READ); outb(VGA_AC_INDEX, idx); outb(VGA_AC_WRITE, val); }

/* ===========================================================================
 * SECCION 3: CAMBIO DE MODO POR PUERTOS (sin BIOS)
 *
 * Tablas extraidas de RBIL, FreeVGA y codigo fuente de VGABIOS (LGPLv2).
 * =========================================================================== */

static void set_mode13h(void)
{
    /* Miscellaneous Output: 25.175 MHz, color I/O base 3Dx */
    outb(VGA_MISC_WRITE, 0x63);

    /* Sequencer — secuencia correcta: sync reset PRIMERO, programar, liberar */
    seq_w(0x00, 0x01);  /* sync reset: detener Sequencer antes de cambiar config */
    seq_w(0x01, 0x01);  /* clocking: 8 dot/char */
    seq_w(0x02, 0x0F);  /* map mask: todos los planos */
    seq_w(0x03, 0x00);  /* char map select */
    seq_w(0x04, 0x0E);  /* mem mode: chain4 + extended (modo 13h lineal) */
    seq_w(0x00, 0x03);  /* liberar reset: operacion normal */

    /* Desbloquear CRTC */
    crtc_w(0x11, crtc_r(0x11) & 0x7F);

    /* CRTC -- timing 320x200 double-scan */
    crtc_w(0x00, 0x5F); crtc_w(0x01, 0x4F); crtc_w(0x02, 0x50);
    crtc_w(0x03, 0x82); crtc_w(0x04, 0x54); crtc_w(0x05, 0x80);
    crtc_w(0x06, 0xBF); crtc_w(0x07, 0x1F); crtc_w(0x08, 0x00);
    crtc_w(0x09, 0x41); /* Max Scan Line: double-scan ON */
    crtc_w(0x0A, 0x00); crtc_w(0x0B, 0x00);
    crtc_w(0x0C, 0x00); crtc_w(0x0D, 0x00);
    crtc_w(0x0E, 0x00); crtc_w(0x0F, 0x00);
    crtc_w(0x10, 0x9C); crtc_w(0x11, 0x8E); crtc_w(0x12, 0x8F);
    crtc_w(0x13, 0x28); crtc_w(0x14, 0x40); crtc_w(0x15, 0x96);
    crtc_w(0x16, 0xB9); crtc_w(0x17, 0xA3); crtc_w(0x18, 0xFF);

    /* Graphics Controller */
    gc_w(0x00, 0x00); gc_w(0x01, 0x00); gc_w(0x02, 0x00);
    gc_w(0x03, 0x00); gc_w(0x04, 0x00); gc_w(0x05, 0x40);
    gc_w(0x06, 0x05); /* modo 256-color, framebuffer en A0000 */
    gc_w(0x07, 0x0F); gc_w(0x08, 0xFF);

    /* Attribute Controller (palette + modo) */
    {
        unsigned char ac[21] = {
            0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
            0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
            0x01,  /* 0x10: Attribute Mode Control -- 256 color, bit6=0 evita espejo */
            0x00,  /* 0x11: Overscan */
            0x0F,  /* 0x12: Color Plane Enable */
            0x00,  /* 0x13: H Pixel Panning */
            0x00   /* 0x14: Color Select */
        };
        int i;
        inb(VGA_INSTAT_READ);
        for (i = 0; i < 21; i++) {
            outb(VGA_AC_INDEX, (unsigned char)i);
            outb(VGA_AC_WRITE, ac[i]);
        }
        inb(VGA_INSTAT_READ);
        outb(VGA_AC_INDEX, 0x20); /* habilitar video */
    }
}

static void set_mode3(void)
{
    /* Miscellaneous Output */
    outb(VGA_MISC_WRITE, 0x67);

    /* Sequencer — secuencia correcta: sync reset PRIMERO, programar, liberar */
    seq_w(0x00, 0x01);  /* sync reset */
    seq_w(0x01, 0x00);  /* 9 dot/char */
    seq_w(0x02, 0x03);  /* planos 0+1 */
    seq_w(0x03, 0x00);  /* char map select */
    seq_w(0x04, 0x02);  /* odd/even, no chain4 (modo texto) */
    seq_w(0x00, 0x03);  /* liberar reset */

    /* Desbloquear CRTC */
    crtc_w(0x11, crtc_r(0x11) & 0x7F);

    /* CRTC -- timing 80x25 texto */
    crtc_w(0x00, 0x5F); crtc_w(0x01, 0x4F); crtc_w(0x02, 0x50);
    crtc_w(0x03, 0x82); crtc_w(0x04, 0x55); crtc_w(0x05, 0x81);
    crtc_w(0x06, 0xBF); crtc_w(0x07, 0x1F); crtc_w(0x08, 0x00);
    crtc_w(0x09, 0x4F); /* Max Scan Line: 16 lineas/char */
    crtc_w(0x0A, 0x0D); crtc_w(0x0B, 0x0E);
    crtc_w(0x0C, 0x00); crtc_w(0x0D, 0x00);
    crtc_w(0x0E, 0x00); crtc_w(0x0F, 0x00);
    crtc_w(0x10, 0x9C); crtc_w(0x11, 0x8E); crtc_w(0x12, 0x8F);
    crtc_w(0x13, 0x28); crtc_w(0x14, 0x1F); crtc_w(0x15, 0x96);
    crtc_w(0x16, 0xB9); crtc_w(0x17, 0xA3); crtc_w(0x18, 0xFF);

    /* Graphics Controller -- texto, odd/even */
    gc_w(0x00, 0x00); gc_w(0x01, 0x00); gc_w(0x02, 0x00);
    gc_w(0x03, 0x00); gc_w(0x04, 0x00); gc_w(0x05, 0x10);
    gc_w(0x06, 0x0E); /* texto, B8000, 32K */
    gc_w(0x07, 0x00); gc_w(0x08, 0xFF);

    /* Attribute Controller -- paleta texto estandar */
    {
        unsigned char ac[21] = {
            0x00,0x01,0x02,0x03,0x04,0x05,0x14,0x07,
            0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
            0x0C,  /* 0x10: texto */
            0x00, 0x0F, 0x08, 0x00
        };
        int i;
        inb(VGA_INSTAT_READ);
        for (i = 0; i < 21; i++) {
            outb(VGA_AC_INDEX, (unsigned char)i);
            outb(VGA_AC_WRITE, ac[i]);
        }
        inb(VGA_INSTAT_READ);
        outb(VGA_AC_INDEX, 0x20);
    }

    /* Limpiar framebuffer texto */
    {
        volatile unsigned short *fb = TEXTFB;
        int i;
        for (i = 0; i < TEXT_COLS * TEXT_ROWS; i++)
            fb[i] = 0x0720; /* gris/negro, espacio */
    }

    /* Cursor en (0,0) */
    crtc_w(0x0F, 0x00);
    crtc_w(0x0E, 0x00);
}

/* ===========================================================================
 * SECCION 4: PALETA DAC (componentes 0-63 por canal)
 * =========================================================================== */

static void dac_set(unsigned char idx, unsigned char r,
                    unsigned char g, unsigned char b)
{
    outb(VGA_DAC_WRITE, idx);
    outb(VGA_DAC_DATA, r);
    outb(VGA_DAC_DATA, g);
    outb(VGA_DAC_DATA, b);
}

/*
 * Colores:
 *  0 = Negro        1 = Azul UG oscuro    2 = Azul UG medio
 *  3 = Amarillo UG  4 = Amarillo claro    7 = Gris claro
 *  8 = Gris oscuro  10 = Verde barra      11 = Verde claro
 * 15 = Blanco       20 = Azul fondo alto  21 = Azul fondo bajo
 */
static void setup_palette(void)
{
    dac_set(0,   0,  0,  0);
    dac_set(1,   0,  7, 37);
    dac_set(2,   0, 15, 47);
    dac_set(3,  63, 54,  0);
    dac_set(4,  63, 62, 30);
    dac_set(7,  40, 40, 40);
    dac_set(8,  15, 15, 15);
    dac_set(10,  0, 48,  0);
    dac_set(11, 20, 60, 20);
    dac_set(15, 63, 63, 63);
    dac_set(20,  0, 20, 40);
    dac_set(21,  0, 25, 50);
}

/* ===========================================================================
 * SECCION 5: PRIMITIVAS DE FRAMEBUFFER
 * =========================================================================== */

static inline void px(int x, int y, unsigned char c)
{
    if ((unsigned)x < VGA13_W && (unsigned)y < VGA13_H)
        VGA13_FB[y * VGA13_W + x] = c;
}

static void fill_rect(int x, int y, int w, int h, unsigned char c)
{
    int dx, dy;
    for (dy = 0; dy < h; dy++)
        for (dx = 0; dx < w; dx++)
            px(x + dx, y + dy, c);
}

static void fb_clear(unsigned char c)
{
    unsigned char *p = VGA13_FB;
    int i;
    for (i = 0; i < VGA13_W * VGA13_H; i++) p[i] = c;
}

static void rect_outline(int x, int y, int w, int h, unsigned char c)
{
    int i;
    for (i = 0; i < w; i++) { px(x+i,y,c); px(x+i,y+h-1,c); }
    for (i = 0; i < h; i++) { px(x,y+i,c); px(x+w-1,y+i,c); }
}

/* ===========================================================================
 * SECCION 6: FONT 8x8
 * =========================================================================== */

static const unsigned char FONT8X8[91][8] = {
    /* 32 ' '  */ {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    /* 33 '!'  */ {0x18,0x3C,0x3C,0x18,0x18,0x00,0x18,0x00},
    /* 34 '"'  */ {0x36,0x36,0x00,0x00,0x00,0x00,0x00,0x00},
    /* 35 '#'  */ {0x36,0x7F,0x36,0x36,0x7F,0x36,0x36,0x00},
    /* 36 '$'  */ {0x0C,0x3E,0x03,0x1E,0x30,0x1F,0x0C,0x00},
    /* 37 '%'  */ {0x00,0x63,0x33,0x18,0x0C,0x66,0x63,0x00},
    /* 38 '&'  */ {0x1C,0x36,0x1C,0x6E,0x3B,0x33,0x6E,0x00},
    /* 39 '\'' */ {0x06,0x06,0x03,0x00,0x00,0x00,0x00,0x00},
    /* 40 '('  */ {0x18,0x0C,0x06,0x06,0x06,0x0C,0x18,0x00},
    /* 41 ')'  */ {0x06,0x0C,0x18,0x18,0x18,0x0C,0x06,0x00},
    /* 42 '*'  */ {0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00},
    /* 43 '+'  */ {0x00,0x0C,0x0C,0x3F,0x0C,0x0C,0x00,0x00},
    /* 44 ','  */ {0x00,0x00,0x00,0x00,0x00,0x0C,0x0C,0x06},
    /* 45 '-'  */ {0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x00},
    /* 46 '.'  */ {0x00,0x00,0x00,0x00,0x00,0x0C,0x0C,0x00},
    /* 47 '/'  */ {0x60,0x30,0x18,0x0C,0x06,0x03,0x01,0x00},
    /* 48 '0'  */ {0x3E,0x63,0x73,0x7B,0x6F,0x67,0x3E,0x00},
    /* 49 '1'  */ {0x0C,0x0E,0x0C,0x0C,0x0C,0x0C,0x3F,0x00},
    /* 50 '2'  */ {0x1E,0x33,0x30,0x1C,0x06,0x33,0x3F,0x00},
    /* 51 '3'  */ {0x1E,0x33,0x30,0x1C,0x30,0x33,0x1E,0x00},
    /* 52 '4'  */ {0x38,0x3C,0x36,0x33,0x7F,0x30,0x78,0x00},
    /* 53 '5'  */ {0x3F,0x03,0x1F,0x30,0x30,0x33,0x1E,0x00},
    /* 54 '6'  */ {0x1C,0x06,0x03,0x1F,0x33,0x33,0x1E,0x00},
    /* 55 '7'  */ {0x3F,0x33,0x30,0x18,0x0C,0x0C,0x0C,0x00},
    /* 56 '8'  */ {0x1E,0x33,0x33,0x1E,0x33,0x33,0x1E,0x00},
    /* 57 '9'  */ {0x1E,0x33,0x33,0x3E,0x30,0x18,0x0E,0x00},
    /* 58 ':'  */ {0x00,0x0C,0x0C,0x00,0x00,0x0C,0x0C,0x00},
    /* 59 ';'  */ {0x00,0x0C,0x0C,0x00,0x00,0x0C,0x0C,0x06},
    /* 60 '<'  */ {0x18,0x0C,0x06,0x03,0x06,0x0C,0x18,0x00},
    /* 61 '='  */ {0x00,0x00,0x3F,0x00,0x00,0x3F,0x00,0x00},
    /* 62 '>'  */ {0x06,0x0C,0x18,0x30,0x18,0x0C,0x06,0x00},
    /* 63 '?'  */ {0x1E,0x33,0x30,0x18,0x0C,0x00,0x0C,0x00},
    /* 64 '@'  */ {0x3E,0x63,0x7B,0x7B,0x7B,0x03,0x1E,0x00},
    /* 65 'A'  */ {0x0C,0x1E,0x33,0x33,0x3F,0x33,0x33,0x00},
    /* 66 'B'  */ {0x3F,0x66,0x66,0x3E,0x66,0x66,0x3F,0x00},
    /* 67 'C'  */ {0x3C,0x66,0x03,0x03,0x03,0x66,0x3C,0x00},
    /* 68 'D'  */ {0x1F,0x36,0x66,0x66,0x66,0x36,0x1F,0x00},
    /* 69 'E'  */ {0x7F,0x46,0x16,0x1E,0x16,0x46,0x7F,0x00},
    /* 70 'F'  */ {0x7F,0x46,0x16,0x1E,0x16,0x06,0x0F,0x00},
    /* 71 'G'  */ {0x3C,0x66,0x03,0x03,0x73,0x66,0x7C,0x00},
    /* 72 'H'  */ {0x33,0x33,0x33,0x3F,0x33,0x33,0x33,0x00},
    /* 73 'I'  */ {0x1E,0x0C,0x0C,0x0C,0x0C,0x0C,0x1E,0x00},
    /* 74 'J'  */ {0x78,0x30,0x30,0x30,0x33,0x33,0x1E,0x00},
    /* 75 'K'  */ {0x67,0x66,0x36,0x1E,0x36,0x66,0x67,0x00},
    /* 76 'L'  */ {0x0F,0x06,0x06,0x06,0x46,0x66,0x7F,0x00},
    /* 77 'M'  */ {0x63,0x77,0x7F,0x7F,0x6B,0x63,0x63,0x00},
    /* 78 'N'  */ {0x63,0x67,0x6F,0x7B,0x73,0x63,0x63,0x00},
    /* 79 'O'  */ {0x1C,0x36,0x63,0x63,0x63,0x36,0x1C,0x00},
    /* 80 'P'  */ {0x3F,0x66,0x66,0x3E,0x06,0x06,0x0F,0x00},
    /* 81 'Q'  */ {0x1E,0x33,0x33,0x33,0x3B,0x1E,0x38,0x00},
    /* 82 'R'  */ {0x3F,0x66,0x66,0x3E,0x36,0x66,0x67,0x00},
    /* 83 'S'  */ {0x1E,0x33,0x07,0x0E,0x38,0x33,0x1E,0x00},
    /* 84 'T'  */ {0x3F,0x2D,0x0C,0x0C,0x0C,0x0C,0x1E,0x00},
    /* 85 'U'  */ {0x33,0x33,0x33,0x33,0x33,0x33,0x3F,0x00},
    /* 86 'V'  */ {0x33,0x33,0x33,0x33,0x33,0x1E,0x0C,0x00},
    /* 87 'W'  */ {0x63,0x63,0x63,0x6B,0x7F,0x77,0x63,0x00},
    /* 88 'X'  */ {0x63,0x63,0x36,0x1C,0x1C,0x36,0x63,0x00},
    /* 89 'Y'  */ {0x33,0x33,0x33,0x1E,0x0C,0x0C,0x1E,0x00},
    /* 90 'Z'  */ {0x7F,0x63,0x31,0x18,0x4C,0x66,0x7F,0x00},
    /* 91 '['  */ {0x1E,0x06,0x06,0x06,0x06,0x06,0x1E,0x00},
    /* 92 '\\' */ {0x03,0x06,0x0C,0x18,0x30,0x60,0x40,0x00},
    /* 93 ']'  */ {0x1E,0x18,0x18,0x18,0x18,0x18,0x1E,0x00},
    /* 94 '^'  */ {0x08,0x1C,0x36,0x63,0x00,0x00,0x00,0x00},
    /* 95 '_'  */ {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF},
    /* 96 '`'  */ {0x0C,0x0C,0x18,0x00,0x00,0x00,0x00,0x00},
    /* 97 'a'  */ {0x00,0x00,0x1E,0x30,0x3E,0x33,0x6E,0x00},
    /* 98 'b'  */ {0x07,0x06,0x06,0x3E,0x66,0x66,0x3B,0x00},
    /* 99 'c'  */ {0x00,0x00,0x1E,0x33,0x03,0x33,0x1E,0x00},
    /*100 'd'  */ {0x38,0x30,0x30,0x3E,0x33,0x33,0x6E,0x00},
    /*101 'e'  */ {0x00,0x00,0x1E,0x33,0x3F,0x03,0x1E,0x00},
    /*102 'f'  */ {0x1C,0x36,0x06,0x0F,0x06,0x06,0x0F,0x00},
    /*103 'g'  */ {0x00,0x00,0x6E,0x33,0x33,0x3E,0x30,0x1F},
    /*104 'h'  */ {0x07,0x06,0x36,0x6E,0x66,0x66,0x67,0x00},
    /*105 'i'  */ {0x0C,0x00,0x0E,0x0C,0x0C,0x0C,0x1E,0x00},
    /*106 'j'  */ {0x30,0x00,0x30,0x30,0x30,0x33,0x33,0x1E},
    /*107 'k'  */ {0x07,0x06,0x66,0x36,0x1E,0x36,0x67,0x00},
    /*108 'l'  */ {0x0E,0x0C,0x0C,0x0C,0x0C,0x0C,0x1E,0x00},
    /*109 'm'  */ {0x00,0x00,0x33,0x7F,0x7F,0x6B,0x63,0x00},
    /*110 'n'  */ {0x00,0x00,0x1F,0x33,0x33,0x33,0x33,0x00},
    /*111 'o'  */ {0x00,0x00,0x1E,0x33,0x33,0x33,0x1E,0x00},
    /*112 'p'  */ {0x00,0x00,0x3B,0x66,0x66,0x3E,0x06,0x0F},
    /*113 'q'  */ {0x00,0x00,0x6E,0x33,0x33,0x3E,0x30,0x78},
    /*114 'r'  */ {0x00,0x00,0x3B,0x6E,0x66,0x06,0x0F,0x00},
    /*115 's'  */ {0x00,0x00,0x3E,0x03,0x1E,0x30,0x1F,0x00},
    /*116 't'  */ {0x08,0x0C,0x3E,0x0C,0x0C,0x2C,0x18,0x00},
    /*117 'u'  */ {0x00,0x00,0x33,0x33,0x33,0x33,0x6E,0x00},
    /*118 'v'  */ {0x00,0x00,0x33,0x33,0x33,0x1E,0x0C,0x00},
    /*119 'w'  */ {0x00,0x00,0x63,0x6B,0x7F,0x7F,0x36,0x00},
    /*120 'x'  */ {0x00,0x00,0x63,0x36,0x1C,0x36,0x63,0x00},
    /*121 'y'  */ {0x00,0x00,0x33,0x33,0x33,0x3E,0x30,0x1F},
    /*122 'z'  */ {0x00,0x00,0x3F,0x19,0x0C,0x26,0x3F,0x00},
};

/* ===========================================================================
 * SECCION 7: PRIMITIVAS DE TEXTO EN MODO GRAFICO
 * =========================================================================== */

static void draw_char(int sx, int sy, char c,
                       unsigned char color, int scale)
{
    int row, col, bx, by;
    const unsigned char *glyph;
    if (c < 32 || c > 122) c = '?';
    glyph = FONT8X8[(unsigned char)(c - 32)];
    for (row = 0; row < 8; row++) {
        unsigned char bits = glyph[row];
        for (col = 0; col < 8; col++) {
            if (bits & (0x80u >> col)) {
                for (by = 0; by < scale; by++)
                    for (bx = 0; bx < scale; bx++)
                        px(sx + col*scale + bx, sy + row*scale + by, color);
            }
        }
    }
}

static int slen(const char *s) { int n=0; while(s[n]) n++; return n; }

static void draw_str(int x, int y, const char *s,
                      unsigned char color, int scale)
{
    while (*s) { draw_char(x, y, *s, color, scale); x += 8*scale; s++; }
}

static void draw_str_c(int y, const char *s,
                        unsigned char color, int scale)
{
    int w = slen(s) * 8 * scale;
    draw_str((VGA13_W - w) / 2, y, s, color, scale);
}

/* ===========================================================================
 * SECCION 8: DELAY SIN BIOS
 * =========================================================================== */

static void delay_ms(unsigned int ms)
{
    /* ~2000 NOPs por ms en QEMU sin KVM. volatile evita que -O2 lo elimine. */
    const unsigned int K = 2000;
    volatile unsigned int i;
    for (i = 0; i < ms * K; i++) __asm__ volatile("nop");
}

/* ===========================================================================
 * SECCION 9: LOGO UG
 * =========================================================================== */

/* isqrt entera sin FPU */
static int isqrt(int n)
{
    int q;
    if (n <= 0) return 0;
    q = (n >> 1) + 1;
    while (q * q > n) q = (q + n/q) >> 1;
    return q;
}

static void fill_ellipse(int cx, int cy, int rx, int ry, unsigned char c)
{
    int r;
    for (r = -ry; r <= ry; r++) {
        int hw = (rx * isqrt(ry*ry - r*r)) / ry;
        int x;
        for (x = cx - hw; x <= cx + hw; x++) px(x, cy+r, c);
    }
}

static void stroke_ellipse(int cx, int cy, int rx, int ry, unsigned char c)
{
    int r;
    for (r = -ry; r <= ry; r++) {
        int hw = (rx * isqrt(ry*ry - r*r)) / ry;
        px(cx-hw,   cy+r, c); px(cx-hw+1, cy+r, c);
        px(cx+hw,   cy+r, c); px(cx+hw-1, cy+r, c);
    }
}

static void draw_logo(void)
{
    int row;
    /* Fondo degradado */
    for (row = 0; row < VGA13_H; row++)
        fill_rect(0, row, VGA13_W, 1, row < 100 ? 1 : 20);

    /* Escudo: elipse exterior amarilla, interior azul */
    stroke_ellipse(160, 68, 55, 44, 3);
    fill_ellipse  (160, 68, 52, 41, 2);

    /* Letras UG en el escudo (escala 2) */
    draw_char(160-20, 68-8, 'U', 3, 2);
    draw_char(160+ 4, 68-8, 'G', 3, 2);

    /* Linea decorativa */
    fill_rect(35, 122, 250, 2, 3);
    fill_rect(35, 125, 250, 1, 4);

    /* Texto institucional */
    draw_str_c(130, "UNIVERSIDAD DE GUAYAQUIL", 15, 1);
    draw_str_c(141, "System Operative Edit  v0.4",  7, 1);

    /* Linea inferior */
    fill_rect(35, 153, 250, 2, 3);
}

/* ===========================================================================
 * SECCION 10: BARRA DE PROGRESO
 * =========================================================================== */

#define BAR_X  60
#define BAR_Y  163
#define BAR_W  200
#define BAR_H  9

static void draw_uint(int x, int y, unsigned int n, unsigned char c)
{
    char buf[12]; int i = 10; buf[11] = '\0';
    if (n == 0) { buf[--i] = '0'; }
    else { while (n > 0) { buf[--i] = '0' + (n % 10); n /= 10; } }
    draw_str(x, y, buf + i, c, 1);
}

static void draw_progress(int step, int total, const char *label)
{
    int filled, pct;
    /* Fondo barra */
    fill_rect(BAR_X-2, BAR_Y-2, BAR_W+4, BAR_H+4, 8);
    fill_rect(BAR_X, BAR_Y, BAR_W, BAR_H, 1);

    filled = (BAR_W * step) / total;
    if (filled > 0) fill_rect(BAR_X, BAR_Y, filled, BAR_H, 10);
    if (filled < BAR_W) fill_rect(BAR_X+filled, BAR_Y, 2, BAR_H, 3);

    rect_outline(BAR_X-1, BAR_Y-1, BAR_W+2, BAR_H+2, 15);

    /* Porcentaje */
    pct = (step * 100) / total;
    fill_rect(BAR_X+BAR_W+5, BAR_Y, 28, 9, 1);
    draw_uint(BAR_X+BAR_W+5, BAR_Y, (unsigned)pct, 15);
    draw_str(BAR_X+BAR_W+5+(pct<10?8:pct<100?16:24), BAR_Y, "%", 15, 1);

    /* Etiqueta */
    fill_rect(35, BAR_Y+BAR_H+5, 250, 10, 1);
    draw_str_c(BAR_Y+BAR_H+5, label, 7, 1);
}

/* ===========================================================================
 * SECCION 11: PUNTO DE ENTRADA PUBLICO
 * =========================================================================== */

void KernelShowBootSplash(void)
{
    int i;
    const char *labels[5] = {
        "Inicializando hardware...",
        "Cargando modulos del kernel...",
        "Configurando memoria...",
        "Preparando drivers VGA...",
        "Iniciando interfaz grafica..."
    };

    /* 1. Modo VGA 13h por puertos (sin BIOS, funciona en modo protegido) */
    set_mode13h();

    /* 2. Paleta institucional UG */
    setup_palette();

    /* 3. Pantalla negra */
    fb_clear(0);
    delay_ms(80);

    /* 4. Logo UG */
    draw_logo();

    /* 5. Zona de barra */
    fill_rect(0, 158, VGA13_W, VGA13_H - 158, 1);

    /* 6. Barra inicial vacia */
    draw_progress(0, 5, "Iniciando...");
    delay_ms(250);

    /* 7. Progreso: 5 fases */
    for (i = 0; i < 5; i++) {
        draw_progress(i + 1, 5, labels[i]);
        delay_ms(420);
    }

    /* 8. Destello verde al completar */
    fill_rect(BAR_X, BAR_Y, BAR_W, BAR_H, 11);
    delay_ms(120);
    draw_progress(5, 5, "Sistema listo!");
    delay_ms(380);

    /* 9. Restaurar modo texto 80x25 por puertos (sin BIOS) */
    set_mode3();
    delay_ms(30);
}
