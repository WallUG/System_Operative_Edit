/*
 * BootLogo - Boot Logo Renderer Header
 * Copyright (c) 2024 System_Operative_Edit Project
 * 
 * Licencia: GPL-3.0
 */

#ifndef _BOOTLOGO_H_
#define _BOOTLOGO_H_

/*
 * BootLogoRender - Renderiza el logo de la Universidad de Guayaquil
 * 
 * @bitmap_data: Puntero a datos BMP (o NULL para usar logo embebido)
 * @size: Tamaño de los datos en bytes
 * 
 * Retorna: 1 si exitoso, 0 si falla
 */
int BootLogoRender(const unsigned char *bitmap_data, unsigned int size);

/*
 * BootLogoDrawText - Dibuja texto en la pantalla
 * 
 * @text: Cadena de texto a dibujar
 * @y: Posición Y del texto
 * @color: Índice de color en la paleta
 */
void BootLogoDrawText(const char *text, int y, unsigned char color);

/*
 * BootLogoDrawProgressBar - Dibuja una barra de progreso gráfica
 * 
 * @progress: Progreso de 0 a 100
 * @y: Posición Y de la barra
 */
void BootLogoDrawProgressBar(int progress, int y);

#endif /* _BOOTLOGO_H_ */
