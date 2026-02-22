#ifndef _TSS_H
#define _TSS_H

#include <types.h>

/* Inicializa la estructura TSS y la carga en el TR.
 * Debe llamarse después de gdt_init() y vmm_init().
 */
void tss_init(void);

/* Actualiza el campo ESP0 del TSS.
 * Se invoca en cada cambio de thread para que el hardware
 * sepa qué pila usar al volver de Ring 3 a Ring 0.
 */
void tss_set_esp0(uint32_t esp0);

#endif /* _TSS_H */
