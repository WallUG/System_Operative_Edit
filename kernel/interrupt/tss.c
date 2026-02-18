/*
 * tss.c — Implementación del TSS + descriptor en GDT
 *
 * El GDT actual tiene 5 entradas (índices 0-4):
 *   0: NULL
 *   1: Kernel Code  (selector 0x08)
 *   2: Kernel Data  (selector 0x10)
 *   3: User Code    (selector 0x18, RPL=3 → 0x1B)
 *   4: User Data    (selector 0x20, RPL=3 → 0x23)
 *
 * Añadimos la entrada 5:
 *   5: TSS          (selector 0x28)
 *
 * El GDT en gdt.c fue declarado con 5 entradas (static struct gdt_entry gdt[5]).
 * Necesitamos expandirlo a 6. Como no podemos redimensionar un array estático
 * externo, el enfoque más limpio es que tss.c maneje su propio descriptor
 * y parche el GDT en memoria directamente, o que gdt.c exponga una función
 * para añadir entradas. Aquí usamos la segunda opción:
 * gdt_install_tss(base, limit) — declarado extern.
 */

#include "tss.h"
#include <types.h>

/* ── TSS global ──────────────────────────────────────────────────────── */
static tss_t g_tss;

/* ── Función en gdt.c que añade el descriptor del TSS ────────────────── */
extern void gdt_install_tss(uint32_t base, uint32_t limit);

/* ── API pública ─────────────────────────────────────────────────────── */

void tss_init(void)
{
    uint32_t base  = (uint32_t)&g_tss;
    uint32_t limit = sizeof(tss_t) - 1;

    /* Limpiar toda la estructura */
    uint8_t* p = (uint8_t*)&g_tss;
    for (uint32_t i = 0; i < sizeof(tss_t); i++) p[i] = 0;

    /* Campos obligatorios:
     * ss0  = selector de datos del kernel (0x10)
     * esp0 = se actualiza antes de cada context switch a Ring 3
     * iomap_base = sizeof(tss_t) → sin IO permission bitmap (todo denegado) */
    g_tss.ss0        = 0x10;
    g_tss.esp0       = 0;            /* se pondrá al hacer el primer switch */
    g_tss.iomap_base = sizeof(tss_t);

    /* Instalar descriptor en el GDT (entrada 5, selector 0x28)
     * Flags del descriptor TSS:
     *   type_attr = 0x89 = 10001001b
     *     bit7=1 (Present), bit6-5=00 (DPL=0), bit4=0 (S=0 sistema),
     *     bit3-0=1001 (tipo=TSS32 disponible)
     * granularity = 0x00 (límite en bytes, no en páginas) */
    gdt_install_tss(base, limit);

    /* Cargar el Task Register con el selector 0x28 (entrada 5 del GDT) */
    __asm__ volatile("ltr %0" :: "r"((uint16_t)0x28));
}

void tss_set_kernel_stack(uint32_t esp0)
{
    g_tss.esp0 = esp0;
}
