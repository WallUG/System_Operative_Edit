/*
 * GDT (Global Descriptor Table) Implementation
 * Ensures correct segmentation for protected mode
 */

#include "types.h"

/* GDT Entry structure */
struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed));

/* GDT Pointer structure */
struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

#include "gdt.h"

/* GDT with 6 entries: null, code0, data0, code3, data3, TSS */
static struct gdt_entry gdt[6];
static struct gdt_ptr gdtp;

/* Set GDT gate */
static void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;
    
    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    
    gdt[num].access = access;
}

/* Instalación de descriptor TSS en la entrada 5 de la GDT.
 * El descriptor debe tener tipo "available 32-bit TSS" (0x9) y DPL=0.
 */
void gdt_install_tss(uint32_t base, uint32_t limit)
{
    /* access: present(1) | DPL=0 | S=0 | type=9 */
    uint8_t access = 0x89;
    /* granularity: byte granularity, 32‑bit (0x40) */
    uint8_t gran = 0x40;
    gdt_set_gate(5, base, limit, access, gran);
}

/* Initialize GDT */
void gdt_init(void)
{
    /* Setup GDT pointer */
    gdtp.limit = (sizeof(struct gdt_entry) * 6) - 1;
    gdtp.base = (uint32_t)&gdt;
    
    /* NULL descriptor */
    gdt_set_gate(0, 0, 0, 0, 0);
    
    /* Code segment (ring 0) */
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    
    /* Data segment (ring 0) */
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
    
    /* Code segment (ring 3) */
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
    
    /* Data segment (ring 3) */
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);
    
    /* TSS descriptor placeholder - rellenado por tss_init() */
    gdt_set_gate(5, 0, 0, 0, 0);
    
    /* Load GDT */
    __asm__ volatile("lgdt %0" : : "m"(gdtp));
    
    /* Reload segment registers.
     * El ljmp recarga CS con el selector 0x08 (kernel code segment).
     * Usamos una direccion absoluta con $1f para evitar problemas de
     * resolucion de labels en inline asm de GCC. */
    __asm__ volatile(
        "mov $0x10, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "mov %%ax, %%ss\n"
        "ljmp $0x08, $1f\n"
        "1:\n"
        ::: "eax"
    );
}