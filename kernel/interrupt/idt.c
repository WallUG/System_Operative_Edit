/*
 * IDT (Interrupt Descriptor Table) Implementation
 */

#include "types.h"

struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  zero;
    uint8_t  type_attr;
    uint16_t offset_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

static struct idt_entry idt[256];
static struct idt_ptr   idtp;

extern void _default_exception_handler(void);
extern void idt_load(uint32_t idt_ptr_addr);

static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags)
{
    idt[num].offset_low  = base & 0xFFFF;
    idt[num].offset_high = (base >> 16) & 0xFFFF;
    idt[num].selector    = sel;
    idt[num].zero        = 0;
    idt[num].type_attr   = flags;
}

void idt_init(void)
{
    int i;
    uint32_t handler = (uint32_t)&_default_exception_handler;

    idtp.limit = (uint16_t)(sizeof(struct idt_entry) * 256 - 1);
    idtp.base  = (uint32_t)&idt[0];

    for (i = 0; i < 256; i++) {
        idt_set_gate((uint8_t)i, handler, 0x08, 0x8E);
    }

    idt_load((uint32_t)&idtp);
}
