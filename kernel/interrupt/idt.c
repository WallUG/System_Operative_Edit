/*
 * IDT (Interrupt Descriptor Table) Implementation
 * Prevents Triple Faults by handling CPU exceptions
 */

#include "types.h"

/* IDT Entry structure */
struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attr;
    uint16_t offset_high;
} __attribute__((packed));

/* IDT Pointer structure */
struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

/* IDT with 256 entries */
static struct idt_entry idt[256];
static struct idt_ptr idtp;

/* Default exception handler */
extern void default_exception_handler(void);

/* Assembly stub for default handler */
__asm__(
    ".global default_exception_handler\n"
    "default_exception_handler:\n"
    "    cli\n"
    "    hlt\n"
    "    jmp default_exception_handler\n"
);

/* Set IDT gate */
static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags)
{
    idt[num].offset_low = base & 0xFFFF;
    idt[num].offset_high = (base >> 16) & 0xFFFF;
    idt[num].selector = sel;
    idt[num].zero = 0;
    idt[num].type_attr = flags;
}

/* Initialize IDT */
void idt_init(void)
{
    int i;
    
    /* Setup IDT pointer */
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (uint32_t)&idt;
    
    /* Clear IDT */
    for (i = 0; i < 256; i++) {
        idt_set_gate(i, (uint32_t)default_exception_handler, 0x08, 0x8E);
    }
    
    /* Load IDT */
    __asm__ volatile("lidt %0" : : "m"(idtp));
}