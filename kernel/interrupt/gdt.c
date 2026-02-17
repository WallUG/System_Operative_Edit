/*
 * GDT (Global Descriptor Table) Implementation
 * Ensures correct segmentation for protected mode
 */

#include <stdint.h>

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

/* GDT with 5 entries: null, code, data, user code, user data */
static struct gdt_entry gdt[5];
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
    gdtp.limit = (sizeof(struct gdt_entry) * 5) - 1;
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
    
    /* Load GDT */
    __asm__ volatile("lgdt %0" : : "m"(gdtp));
    
    /* Reload segment registers */
    __asm__ volatile(
        "mov $0x10, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "mov %%ax, %%ss\n"
        "ljmp $0x08, $flush\n"
        "flush:\n"
        ::: "eax"
    );
}