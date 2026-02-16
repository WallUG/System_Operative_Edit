/*
 * FreeLoader - Simplified Boot Loader
 * Copyright (c) 2024 System_Operative_Edit Project
 * 
 * Este archivo es parte del proyecto System_Operative_Edit
 * Licencia: GPL-3.0
 * 
 * string.c - Funciones básicas de strings
 * 
 * Implementación de funciones de string sin usar libc estándar
 * para el entorno del bootloader
 */

#include "include/freeldr.h"

/*
 * strlen - Calcula la longitud de una cadena
 * @str: Cadena a medir
 * Retorna: Longitud de la cadena (sin contar el '\0')
 */
int strlen(const char *str)
{
    int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

/*
 * strcpy - Copia una cadena
 * @dest: Destino
 * @src: Origen
 */
void strcpy(char *dest, const char *src)
{
    int i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

/*
 * strcmp - Compara dos cadenas
 * @s1: Primera cadena
 * @s2: Segunda cadena
 * Retorna: 0 si son iguales, <0 si s1 < s2, >0 si s1 > s2
 */
int strcmp(const char *s1, const char *s2)
{
    int i = 0;
    while (s1[i] != '\0' && s2[i] != '\0') {
        if (s1[i] != s2[i]) {
            return s1[i] - s2[i];
        }
        i++;
    }
    return s1[i] - s2[i];
}

/*
 * memcpy - Copia bloques de memoria
 * @dest: Destino
 * @src: Origen
 * @n: Número de bytes a copiar
 */
void memcpy(void *dest, const void *src, int n)
{
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    
    for (int i = 0; i < n; i++) {
        d[i] = s[i];
    }
}

/*
 * memset - Llena un bloque de memoria con un valor
 * @ptr: Puntero al bloque de memoria
 * @value: Valor a escribir
 * @n: Número de bytes a llenar
 */
void memset(void *ptr, int value, int n)
{
    unsigned char *p = (unsigned char *)ptr;
    unsigned char val = (unsigned char)value;
    
    for (int i = 0; i < n; i++) {
        p[i] = val;
    }
}

/*
 * memcmp - Compara dos bloques de memoria
 * @s1: Primer bloque
 * @s2: Segundo bloque
 * @n: Número de bytes a comparar
 * Retorna: 0 si son iguales, <0 si s1 < s2, >0 si s1 > s2
 */
int memcmp(const void *s1, const void *s2, int n)
{
    const unsigned char *p1 = (const unsigned char *)s1;
    const unsigned char *p2 = (const unsigned char *)s2;
    
    for (int i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] - p2[i];
        }
    }
    return 0;
}
