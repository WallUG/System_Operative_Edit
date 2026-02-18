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

/* GDT with 6 entries: null, kernel code, kernel data,
 *                     user code, user data, TSS */
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
    
    /* Entry 5 (TSS) — se rellena después via gdt_install_tss() */
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

/*
 * gdt_install_tss — rellena la entrada 5 del GDT con el descriptor del TSS.
 *
 * El descriptor de un TSS es de tipo "sistema" (S=0) con tipo=0x9 (TSS32 libre).
 * access byte: 0x89 = Present(1) | DPL=0(00) | S=0 | Type=1001
 * granularity: 0x00 (limite en bytes, no en paginas de 4KB)
 *
 * Llamado desde tss_init() DESPUES de gdt_init().
 */
void gdt_install_tss(uint32_t base, uint32_t limit)
{
    /* Rellenar entrada 5 directamente */
    gdt[5].base_low    = base & 0xFFFF;
    gdt[5].base_middle = (base >> 16) & 0xFF;
    gdt[5].base_high   = (base >> 24) & 0xFF;
    gdt[5].limit_low   = limit & 0xFFFF;
    gdt[5].granularity = (limit >> 16) & 0x0F; /* granularidad=0, bits altos del limite */
    gdt[5].access      = 0x89;  /* Present, DPL=0, S=0, Type=TSS32-available */

    /* Recargar el GDTR para que el CPU vea los cambios */
    __asm__ volatile("lgdt %0" :: "m"(gdtp));
}