/*
 * BootLogo - Boot Logo Renderer
 * Copyright (c) 2024 System_Operative_Edit Project
 * 
 * Este archivo maneja la carga y renderizado del logo BMP
 * de la Universidad de Guayaquil
 * 
 * Licencia: GPL-3.0
 */

#include "../include/bootvid.h"
#include "../include/bootlogo.h"

/* Define NULL if not already defined */
#ifndef NULL
#define NULL ((void*)0)
#endif

/* Estructura simplificada de encabezado BMP */
typedef struct {
    unsigned short bfType;          /* 'BM' */
    unsigned int bfSize;            /* Tamaño del archivo */
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    unsigned int bfOffBits;         /* Offset a los datos de imagen */
} __attribute__((packed)) BMP_FILE_HEADER;

typedef struct {
    unsigned int biSize;            /* Tamaño de este header */
    int biWidth;                    /* Ancho de la imagen */
    int biHeight;                   /* Alto de la imagen */
    unsigned short biPlanes;        /* Número de planos */
    unsigned short biBitCount;      /* Bits por píxel */
    unsigned int biCompression;     /* Tipo de compresión */
    unsigned int biSizeImage;       /* Tamaño de la imagen */
    int biXPelsPerMeter;
    int biYPelsPerMeter;
    unsigned int biClrUsed;
    unsigned int biClrImportant;
} __attribute__((packed)) BMP_INFO_HEADER;

/*
 * Logo UG simplificado en formato de datos embebidos
 * Si no se puede cargar un BMP desde disco, usamos este logo
 */
static void draw_embedded_logo(int center_x, int center_y)
{
    /* Dimensiones del logo simplificado */
    const int logo_width = 100;
    const int logo_height = 80;
    
    int x = center_x - logo_width / 2;
    int y = center_y - logo_height / 2;
    
    /* Fondo del logo (azul institucional) */
    BootVidDrawRect(x, y, logo_width, logo_height, COLOR_BLUE_LIGHT);
    
    /* Borde amarillo */
    BootVidDrawRectOutline(x, y, logo_width, logo_height, COLOR_YELLOW);
    BootVidDrawRectOutline(x + 1, y + 1, logo_width - 2, logo_height - 2, COLOR_YELLOW);
    
    /* Letras "UG" simplificadas usando rectángulos */
    int letter_start_x = x + 20;
    int letter_y = y + 20;
    int letter_height = 40;
    int letter_width = 6;
    int letter_spacing = 20;
    
    /* Letra U */
    /* Barra izquierda */
    BootVidDrawRect(letter_start_x, letter_y, letter_width, letter_height, COLOR_YELLOW);
    /* Barra derecha */
    BootVidDrawRect(letter_start_x + 14, letter_y, letter_width, letter_height, COLOR_YELLOW);
    /* Parte inferior */
    BootVidDrawRect(letter_start_x, letter_y + letter_height - letter_width, 20, letter_width, COLOR_YELLOW);
    
    /* Letra G */
    letter_start_x += 40;
    /* Barra izquierda */
    BootVidDrawRect(letter_start_x, letter_y, letter_width, letter_height, COLOR_YELLOW);
    /* Parte superior */
    BootVidDrawRect(letter_start_x, letter_y, 20, letter_width, COLOR_YELLOW);
    /* Parte inferior */
    BootVidDrawRect(letter_start_x, letter_y + letter_height - letter_width, 20, letter_width, COLOR_YELLOW);
    /* Barra derecha corta */
    BootVidDrawRect(letter_start_x + 14, letter_y + letter_height / 2, letter_width, letter_height / 2, COLOR_YELLOW);
    /* Barra horizontal media */
    BootVidDrawRect(letter_start_x + 10, letter_y + letter_height / 2, 10, letter_width, COLOR_YELLOW);
}

/*
 * BootLogoRender - Renderiza el logo en el centro de la pantalla
 * 
 * @bitmap_data: Datos del bitmap BMP (puede ser NULL para usar logo embebido)
 * @size: Tamaño de los datos
 * 
 * Retorna: 1 si exitoso, 0 si falla
 */
int BootLogoRender(const unsigned char *bitmap_data, unsigned int size)
{
    if (!BootVidIsActive()) {
        return 0;
    }
    
    /* Calcular posición central */
    int center_x = BOOTVID_WIDTH / 2;
    int center_y = BOOTVID_HEIGHT / 2;
    
    /* Si no hay datos BMP, usar logo embebido */
    if (bitmap_data == NULL || size == 0) {
        draw_embedded_logo(center_x, center_y);
        return 1;
    }
    
    /* Parsear el BMP */
    BMP_FILE_HEADER *file_header = (BMP_FILE_HEADER *)bitmap_data;
    
    /* Verificar firma BMP */
    if (file_header->bfType != 0x4D42) { /* 'BM' */
        /* Si el BMP es inválido, usar logo embebido */
        draw_embedded_logo(center_x, center_y);
        return 1;
    }
    
    BMP_INFO_HEADER *info_header = (BMP_INFO_HEADER *)(bitmap_data + sizeof(BMP_FILE_HEADER));
    
    /* Verificar que es un BMP de 8 bits sin compresión */
    if (info_header->biBitCount != 8 || info_header->biCompression != 0) {
        draw_embedded_logo(center_x, center_y);
        return 1;
    }
    
    /* Obtener dimensiones */
    int width = info_header->biWidth;
    int height = info_header->biHeight > 0 ? info_header->biHeight : -info_header->biHeight;
    int bottom_up = info_header->biHeight > 0;
    
    /* Calcular posición para centrar */
    int start_x = center_x - width / 2;
    int start_y = center_y - height / 2;
    
    /* Obtener puntero a datos de imagen */
    const unsigned char *image_data = bitmap_data + file_header->bfOffBits;
    
    /* Calcular padding de fila (las filas BMP están alineadas a 4 bytes) */
    int row_padding = (4 - (width % 4)) % 4;
    
    /* Renderizar el bitmap */
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int src_y = bottom_up ? (height - 1 - y) : y;
            int offset = src_y * (width + row_padding) + x;
            unsigned char color = image_data[offset];
            
            /* Dibujar el píxel si está dentro de los límites */
            int screen_x = start_x + x;
            int screen_y = start_y + y;
            
            if (screen_x >= 0 && screen_x < BOOTVID_WIDTH &&
                screen_y >= 0 && screen_y < BOOTVID_HEIGHT) {
                BootVidSetPixel(screen_x, screen_y, color);
            }
        }
    }
    
    return 1;
}

/*
 * BootLogoDrawText - Dibuja texto simple debajo del logo
 * 
 * @text: Cadena de texto a dibujar
 * @y: Posición Y del texto
 * @color: Color del texto
 * 
 * Nota: Esta es una implementación muy básica de renderizado de texto
 * usando un font bitmap simple 8x8
 */
void BootLogoDrawText(const char *text, int y, unsigned char color)
{
    if (!BootVidIsActive() || text == NULL) {
        return;
    }
    
    /* Calcular posición X centrada (aproximada) */
    int len = 0;
    while (text[len] != '\0') len++;
    
    int char_width = 8;
    int x = (BOOTVID_WIDTH - (len * char_width)) / 2;
    
    /* Dibujar cada carácter como un rectángulo simple */
    /* Esta es una implementación placeholder */
    for (int i = 0; text[i] != '\0'; i++) {
        /* Por ahora, solo dibujamos un bloque por carácter */
        /* En una implementación completa, aquí iría un bitmap font */
        BootVidDrawRect(x + i * char_width, y, char_width - 1, 8, color);
    }
}

/*
 * BootLogoDrawProgressBar - Dibuja una barra de progreso
 * 
 * @progress: Progreso de 0 a 100
 * @y: Posición Y de la barra
 */
void BootLogoDrawProgressBar(int progress, int y)
{
    if (!BootVidIsActive()) {
        return;
    }
    
    const int bar_width = 200;
    const int bar_height = 10;
    int x = (BOOTVID_WIDTH - bar_width) / 2;
    
    /* Borde de la barra */
    BootVidDrawRectOutline(x - 1, y - 1, bar_width + 2, bar_height + 2, COLOR_WHITE);
    
    /* Fondo de la barra */
    BootVidDrawRect(x, y, bar_width, bar_height, COLOR_GRAY_DARK);
    
    /* Calcular ancho del progreso */
    int filled_width = (bar_width * progress) / 100;
    
    /* Dibujar progreso */
    if (filled_width > 0) {
        BootVidDrawRect(x, y, filled_width, bar_height, COLOR_GREEN);
    }
}
