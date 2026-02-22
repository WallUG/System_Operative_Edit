#ifndef _GDT_H
#define _GDT_H

#include <types.h>

/* Inicializa la GDT básica con segmentos de kernel y usuario.
 * Debe llamarse antes de cualquier uso de la segmentación.
 */
void gdt_init(void);

/* Instala un descriptor de TSS en la GDT en el índice 5 (selector 0x28).
 * base: dirección física/virtual de la estructura TSS.
 * limit: tamaño de la TSS menos uno.
 */
void gdt_install_tss(uint32_t base, uint32_t limit);

#endif /* _GDT_H */
