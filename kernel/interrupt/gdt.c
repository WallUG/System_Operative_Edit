/*
 * gdt.c - Inicialización del GDT
 * La tabla real está definida en gdt_flush.asm para garantizar
 * que los valores son correctos en todos los hipervisores.
 */
#include "types.h"

extern void gdt_flush(uint32_t ignored);

void gdt_init(void)
{
    gdt_flush(0);   /* La tabla y el puntero están en el ASM */
}
