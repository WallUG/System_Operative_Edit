/*
 * BootVid - Boot Video Driver
 * Copyright (c) 2024 System_Operative_Edit Project
 * Based on ReactOS BootVid architecture
 * 
 * Este archivo implementa el driver de video para el bootloader
 * Soporta modo VGA 13h (320x200, 256 colores) para animaciones gráficas
 * 
 * Licencia: GPL-3.0
 */

#include "../include/bootvid.h"

/* VGA Mode 13h constants */
#define VGA_MODE_13H        0x13
#define VGA_WIDTH           320
#define VGA_HEIGHT          200
#define VGA_MEMORY          ((unsigned char*)0xA0000)

/* BIOS interrupt numbers */
#define BIOS_VIDEO_INT      0x10

/* Global state */
static int graphics_mode_active = 0;
static unsigned char saved_mode = 0x03; /* Default text mode */

/*
 * Establece el modo de video usando BIOS interrupts
 */
static void set_video_mode(unsigned char mode)
{
    __asm__ volatile (
        "int $0x10"
        :
        : "a" ((unsigned short)mode)
    );
}

/*
 * Obtiene el modo de video actual
 */
static unsigned char get_video_mode(void)
{
    unsigned char mode;
    __asm__ volatile (
        "int $0x10"
        : "=a" (mode)
        : "a" (0x0F00)
    );
    return mode & 0xFF;
}

/*
 * BootVidInitialize - Inicializa el sistema de video gráfico
 * 
 * Guarda el modo actual y cambia a modo VGA 13h
 * 
 * Retorna: 1 si exitoso, 0 si falla
 */
int BootVidInitialize(void)
{
    /* Guardar modo actual para poder restaurarlo */
    saved_mode = get_video_mode();
    
    /* Cambiar a modo VGA 13h (320x200, 256 colores) */
    set_video_mode(VGA_MODE_13H);
    
    /* Verificar que el cambio fue exitoso */
    if (get_video_mode() == VGA_MODE_13H) {
        graphics_mode_active = 1;
        return 1;
    }
    
    return 0;
}

/*
 * BootVidResetDisplay - Restaura el modo de video original
 */
void BootVidResetDisplay(void)
{
    if (graphics_mode_active) {
        set_video_mode(saved_mode);
        graphics_mode_active = 0;
    }
}

/*
 * BootVidClearScreen - Limpia la pantalla con un color
 * 
 * @color: Índice de color de la paleta VGA (0-255)
 */
void BootVidClearScreen(unsigned char color)
{
    if (!graphics_mode_active) return;
    
    unsigned char *vga = VGA_MEMORY;
    int total_pixels = VGA_WIDTH * VGA_HEIGHT;
    int i;
    
    for (i = 0; i < total_pixels; i++) {
        vga[i] = color;
    }
}

/*
 * BootVidSetPixel - Dibuja un píxel en la pantalla
 * 
 * @x: Coordenada X (0-319)
 * @y: Coordenada Y (0-199)
 * @color: Índice de color de la paleta VGA (0-255)
 */
void BootVidSetPixel(int x, int y, unsigned char color)
{
    if (!graphics_mode_active) return;
    if (x < 0 || x >= VGA_WIDTH || y < 0 || y >= VGA_HEIGHT) return;
    
    VGA_MEMORY[y * VGA_WIDTH + x] = color;
}

/*
 * BootVidDrawRect - Dibuja un rectángulo relleno
 * 
 * @x, @y: Esquina superior izquierda
 * @width, @height: Dimensiones del rectángulo
 * @color: Color de relleno
 */
void BootVidDrawRect(int x, int y, int width, int height, unsigned char color)
{
    int dx, dy;
    
    if (!graphics_mode_active) return;
    
    for (dy = 0; dy < height; dy++) {
        for (dx = 0; dx < width; dx++) {
            BootVidSetPixel(x + dx, y + dy, color);
        }
    }
}

/*
 * BootVidDrawRectOutline - Dibuja el contorno de un rectángulo
 */
void BootVidDrawRectOutline(int x, int y, int width, int height, unsigned char color)
{
    int dx, dy;
    
    if (!graphics_mode_active) return;
    
    /* Líneas horizontales */
    for (dx = 0; dx < width; dx++) {
        BootVidSetPixel(x + dx, y, color);
        BootVidSetPixel(x + dx, y + height - 1, color);
    }
    
    /* Líneas verticales */
    for (dy = 0; dy < height; dy++) {
        BootVidSetPixel(x, y + dy, color);
        BootVidSetPixel(x + width - 1, y + dy, color);
    }
}

/*
 * BootVidSetPaletteColor - Establece un color en la paleta VGA
 * 
 * @index: Índice de color (0-255)
 * @r, @g, @b: Componentes RGB (0-63)
 */
void BootVidSetPaletteColor(unsigned char index, unsigned char r, unsigned char g, unsigned char b)
{
    if (!graphics_mode_active) return;
    
    /* Escribir en los registros de paleta VGA */
    /* Port 0x3C8: Palette Index (write) */
    /* Port 0x3C9: Palette Data */
    __asm__ volatile (
        "outb %%al, %%dx\n"
        :
        : "a" (index), "d" (0x3C8)
    );
    
    __asm__ volatile (
        "outb %%al, %%dx\n"
        :
        : "a" (r), "d" (0x3C9)
    );
    
    __asm__ volatile (
        "outb %%al, %%dx\n"
        :
        : "a" (g), "d" (0x3C9)
    );
    
    __asm__ volatile (
        "outb %%al, %%dx\n"
        :
        : "a" (b), "d" (0x3C9)
    );
}

/*
 * BootVidInitializePalette - Inicializa la paleta con colores institucionales
 * 
 * Configura una paleta con colores de la Universidad de Guayaquil:
 * - Azul institucional
 * - Amarillo institucional
 * - Blanco, negro, grises
 */
void BootVidInitializePalette(void)
{
    if (!graphics_mode_active) return;
    
    /* Color 0: Negro */
    BootVidSetPaletteColor(0, 0, 0, 0);
    
    /* Color 1: Azul oscuro (institucional) */
    BootVidSetPaletteColor(1, 0, 0, 42);
    
    /* Color 2: Azul claro (institucional) */
    BootVidSetPaletteColor(2, 0, 25, 63);
    
    /* Color 3: Amarillo (institucional) */
    BootVidSetPaletteColor(3, 63, 63, 0);
    
    /* Color 4: Amarillo claro */
    BootVidSetPaletteColor(4, 63, 63, 42);
    
    /* Color 15: Blanco */
    BootVidSetPaletteColor(15, 63, 63, 63);
    
    /* Color 7: Gris claro */
    BootVidSetPaletteColor(7, 42, 42, 42);
    
    /* Color 8: Gris oscuro */
    BootVidSetPaletteColor(8, 21, 21, 21);
    
    /* Color 10: Verde (para progress bar) */
    BootVidSetPaletteColor(10, 0, 63, 0);
}

/*
 * BootVidFadeScreen - Aplica efecto de fade in o fade out
 * 
 * @fade_in: 1 para fade in, 0 para fade out
 * @steps: Número de pasos del fade (más = más suave)
 */
void BootVidFadeScreen(int fade_in, int steps)
{
    int i, step;
    unsigned char r, g, b;
    
    if (!graphics_mode_active) return;
    
    /* Guardar paleta original */
    unsigned char palette[256][3];
    
    /* Leer paleta actual */
    for (i = 0; i < 256; i++) {
        /* Port 0x3C7: Palette Index (read) */
        /* Port 0x3C9: Palette Data */
        __asm__ volatile (
            "outb %%al, %%dx\n"
            :
            : "a" ((unsigned char)i), "d" (0x3C7)
        );
        
        __asm__ volatile (
            "inb %%dx, %%al\n"
            : "=a" (palette[i][0])
            : "d" (0x3C9)
        );
        
        __asm__ volatile (
            "inb %%dx, %%al\n"
            : "=a" (palette[i][1])
            : "d" (0x3C9)
        );
        
        __asm__ volatile (
            "inb %%dx, %%al\n"
            : "=a" (palette[i][2])
            : "d" (0x3C9)
        );
    }
    
    /* Aplicar fade */
    for (step = 0; step < steps; step++) {
        for (i = 0; i < 256; i++) {
            if (fade_in) {
                /* Fade in: de negro a color original */
                r = (palette[i][0] * step) / steps;
                g = (palette[i][1] * step) / steps;
                b = (palette[i][2] * step) / steps;
            } else {
                /* Fade out: de color original a negro */
                r = (palette[i][0] * (steps - step)) / steps;
                g = (palette[i][1] * (steps - step)) / steps;
                b = (palette[i][2] * (steps - step)) / steps;
            }
            
            BootVidSetPaletteColor(i, r, g, b);
        }
        
        /* Delay para hacer visible el efecto */
        {
            volatile int delay;
            for (delay = 0; delay < 100000; delay++);
        }
    }
    
    /* Restaurar paleta original al final del fade in */
    if (fade_in) {
        for (i = 0; i < 256; i++) {
            BootVidSetPaletteColor(i, palette[i][0], palette[i][1], palette[i][2]);
        }
    }
}

/*
 * BootVidIsActive - Verifica si el modo gráfico está activo
 */
int BootVidIsActive(void)
{
    return graphics_mode_active;
}
