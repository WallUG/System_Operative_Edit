/*
 * PROJECT:     System_Operative_Edit
 * LICENSE:     GPL-3.0
 * PURPOSE:     Kernel Standard Library Header
 * COPYRIGHT:   Universidad de Guayaquil
 */

#ifndef _KSTDLIB_H
#define _KSTDLIB_H

#include "types.h"

/* Memory management */
void heap_init(void);   /* Llamar UNA vez antes del primer malloc() */
void *malloc(size_t size);
void free(void *ptr);
void *memset(void *ptr, int value, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);

/* String functions */
size_t strlen(const char *str);
int strcmp(const char *s1, const char *s2);
char *strcpy(char *dest, const char *src);

#endif /* _KSTDLIB_H */
