/*
 * BootVid - Boot Video Driver Header
 * Copyright (c) 2024 System_Operative_Edit Project
 * Based on ReactOS BootVid architecture
 * 
 * Licencia: GPL-3.0
 */

#ifndef _BOOTVID_H_
#define _BOOTVID_H_

/* VGA Mode 13h specifications */
#define BOOTVID_WIDTH   320
#define BOOTVID_HEIGHT  200
#define BOOTVID_BPP     8    /* 8 bits per pixel = 256 colors */

/* Colores institucionales UG en la paleta */
#define COLOR_BLACK         0
#define COLOR_BLUE_DARK     1
#define COLOR_BLUE_LIGHT    2
#define COLOR_YELLOW        3
#define COLOR_YELLOW_LIGHT  4
#define COLOR_WHITE         15
#define COLOR_GRAY_LIGHT    7
#define COLOR_GRAY_DARK     8
#define COLOR_GREEN         10

/*
 * Inicialización y limpieza
 */

/* Inicializa el sistema de video gráfico */
int BootVidInitialize(void);

/* Restaura el modo de video original */
void BootVidResetDisplay(void);

/* Verifica si el modo gráfico está activo */
int BootVidIsActive(void);

/*
 * Funciones de dibujo básicas
 */

/* Limpia la pantalla con un color */
void BootVidClearScreen(unsigned char color);

/* Dibuja un píxel */
void BootVidSetPixel(int x, int y, unsigned char color);

/* Dibuja un rectángulo relleno */
void BootVidDrawRect(int x, int y, int width, int height, unsigned char color);

/* Dibuja el contorno de un rectángulo */
void BootVidDrawRectOutline(int x, int y, int width, int height, unsigned char color);

/*
 * Gestión de paleta
 */

/* Establece un color en la paleta VGA */
void BootVidSetPaletteColor(unsigned char index, unsigned char r, unsigned char g, unsigned char b);

/* Inicializa la paleta con colores institucionales */
void BootVidInitializePalette(void);

/*
 * Efectos visuales
 */

/* Aplica efecto de fade in/out */
void BootVidFadeScreen(int fade_in, int steps);

#endif /* _BOOTVID_H_ */
