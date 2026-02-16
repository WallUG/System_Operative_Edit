/*
 * FreeLoader - Simplified Boot Loader
 * Copyright (c) 2024 System_Operative_Edit Project
 * 
 * Este archivo es parte del proyecto System_Operative_Edit
 * Licencia: GPL-3.0
 * 
 * video.h - Funciones de video en modo texto
 */

#ifndef _VIDEO_H
#define _VIDEO_H

#include "freeldr.h"

/* Colores de texto (modo VGA texto) */
#define COLOR_BLACK        0x0
#define COLOR_BLUE         0x1
#define COLOR_GREEN        0x2
#define COLOR_CYAN         0x3
#define COLOR_RED          0x4
#define COLOR_MAGENTA      0x5
#define COLOR_BROWN        0x6
#define COLOR_LIGHT_GRAY   0x7
#define COLOR_DARK_GRAY    0x8
#define COLOR_LIGHT_BLUE   0x9
#define COLOR_LIGHT_GREEN  0xA
#define COLOR_LIGHT_CYAN   0xB
#define COLOR_LIGHT_RED    0xC
#define COLOR_LIGHT_MAGENTA 0xD
#define COLOR_YELLOW       0xE
#define COLOR_WHITE        0xF

/* Macro para crear atributo de color */
#define MAKE_COLOR(fg, bg) ((bg << 4) | (fg))

/* Funciones de video */
void VideoInit(void);
void VideoClearScreen(void);
void VideoPutChar(char c);
void VideoPutString(const char *str);
void VideoSetCursor(int x, int y);
void VideoSetColor(u8 color);
void VideoPutStringAt(int x, int y, const char *str);

#endif /* _VIDEO_H */
