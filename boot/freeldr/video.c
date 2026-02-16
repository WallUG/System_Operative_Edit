/*
 * FreeLoader - Simplified Boot Loader
 * Copyright (c) 2024 System_Operative_Edit Project
 * 
 * Este archivo es parte del proyecto System_Operative_Edit
 * Licencia: GPL-3.0
 * 
 * video.c - Funciones de video en modo texto
 * 
 * Implementa funciones básicas de salida de video usando:
 * - Acceso directo a memoria de video en 0xB8000 (modo texto VGA)
 * - INT 10h del BIOS para funciones avanzadas
 */

#include "include/freeldr.h"
#include "include/video.h"

/* Variables estáticas del módulo de video */
static u16 *video_memory = (u16 *)VIDEO_BUFFER;
static int cursor_x = 0;
static int cursor_y = 0;
static u8 current_color = MAKE_COLOR(COLOR_LIGHT_GRAY, COLOR_BLACK);

/*
 * VideoInit - Inicializa el subsistema de video
 * 
 * Configura el modo de video y limpia la pantalla
 */
void VideoInit(void)
{
    cursor_x = 0;
    cursor_y = 0;
    current_color = MAKE_COLOR(COLOR_LIGHT_GRAY, COLOR_BLACK);
    VideoClearScreen();
}

/*
 * VideoClearScreen - Limpia la pantalla
 * 
 * Llena toda la pantalla con espacios y el color actual
 */
void VideoClearScreen(void)
{
    u16 blank = (current_color << 8) | ' ';
    
    for (int i = 0; i < VIDEO_COLS * VIDEO_ROWS; i++) {
        video_memory[i] = blank;
    }
    
    cursor_x = 0;
    cursor_y = 0;
    VideoSetCursor(0, 0);
}

/*
 * VideoScroll - Desplaza la pantalla una línea hacia arriba
 */
static void VideoScroll(void)
{
    u16 blank = (current_color << 8) | ' ';
    
    // Copiar todas las líneas una posición hacia arriba
    for (int i = 0; i < (VIDEO_ROWS - 1) * VIDEO_COLS; i++) {
        video_memory[i] = video_memory[i + VIDEO_COLS];
    }
    
    // Limpiar la última línea
    for (int i = (VIDEO_ROWS - 1) * VIDEO_COLS; i < VIDEO_ROWS * VIDEO_COLS; i++) {
        video_memory[i] = blank;
    }
}

/*
 * VideoPutChar - Escribe un carácter en la posición actual del cursor
 * @c: Carácter a escribir
 * 
 * Maneja caracteres especiales como '\n', '\r', '\t', '\b'
 */
void VideoPutChar(char c)
{
    u16 attrib = current_color << 8;
    
    // Manejar caracteres especiales
    switch (c) {
        case '\n':  // Nueva línea
            cursor_x = 0;
            cursor_y++;
            break;
            
        case '\r':  // Retorno de carro
            cursor_x = 0;
            break;
            
        case '\t':  // Tabulación (8 espacios)
            cursor_x = (cursor_x + 8) & ~(8 - 1);
            break;
            
        case '\b':  // Backspace
            if (cursor_x > 0) {
                cursor_x--;
                video_memory[cursor_y * VIDEO_COLS + cursor_x] = attrib | ' ';
            }
            break;
            
        default:    // Carácter normal
            if (c >= ' ') {  // Solo caracteres imprimibles
                video_memory[cursor_y * VIDEO_COLS + cursor_x] = attrib | c;
                cursor_x++;
            }
            break;
    }
    
    // Si llegamos al final de la línea, pasar a la siguiente
    if (cursor_x >= VIDEO_COLS) {
        cursor_x = 0;
        cursor_y++;
    }
    
    // Si llegamos al final de la pantalla, hacer scroll
    if (cursor_y >= VIDEO_ROWS) {
        VideoScroll();
        cursor_y = VIDEO_ROWS - 1;
    }
    
    // Actualizar posición del cursor de hardware
    VideoSetCursor(cursor_x, cursor_y);
}

/*
 * VideoPutString - Escribe una cadena en pantalla
 * @str: Cadena a escribir (terminada en '\0')
 */
void VideoPutString(const char *str)
{
    while (*str) {
        VideoPutChar(*str);
        str++;
    }
}

/*
 * VideoSetCursor - Establece la posición del cursor de hardware
 * @x: Columna (0-79)
 * @y: Fila (0-24)
 * 
 * Usa INT 10h del BIOS o acceso directo a puertos VGA
 */
void VideoSetCursor(int x, int y)
{
    cursor_x = x;
    cursor_y = y;
    
    u16 position = y * VIDEO_COLS + x;
    
    // Usar inline assembly para actualizar el cursor de hardware
    // Puerto 0x3D4: Registro de índice del controlador CRT
    // Puerto 0x3D5: Registro de datos del controlador CRT
    __asm__ volatile (
        "movb $0x0F, %%al\n"      // Registro: Cursor Location Low
        "movw $0x3D4, %%dx\n"
        "outb %%al, %%dx\n"
        "movw %0, %%ax\n"
        "movw $0x3D5, %%dx\n"
        "outb %%al, %%dx\n"       // Escribir byte bajo
        
        "movb $0x0E, %%al\n"      // Registro: Cursor Location High
        "movw $0x3D4, %%dx\n"
        "outb %%al, %%dx\n"
        "movw %0, %%ax\n"
        "shrw $8, %%ax\n"
        "movw $0x3D5, %%dx\n"
        "outb %%al, %%dx\n"       // Escribir byte alto
        :
        : "r"(position)
        : "ax", "dx"
    );
}

/*
 * VideoSetColor - Establece el color actual para texto
 * @color: Color (combinación de foreground y background)
 */
void VideoSetColor(u8 color)
{
    current_color = color;
}

/*
 * VideoPutStringAt - Escribe una cadena en una posición específica
 * @x: Columna inicial
 * @y: Fila
 * @str: Cadena a escribir
 */
void VideoPutStringAt(int x, int y, const char *str)
{
    VideoSetCursor(x, y);
    VideoPutString(str);
}
