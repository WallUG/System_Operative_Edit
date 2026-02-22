#include "tss.h"
#include "gdt.h"

/* TSS structure para modo protegido 32‑bit. Sólo rellenamos los campos que
 * necesitamos (esp0 + ss0). El resto se inicializa a 0.
 */
struct tss_entry {
    uint32_t prev_tss;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt_selector;
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((packed));

static struct tss_entry g_tss;

void tss_init(void)
{
    /* Limpiar la TSS */
    for (int i = 0; i < (int)sizeof(g_tss); i++)
        ((char*)&g_tss)[i] = 0;

    /* Segmento de datos de kernel (selector 0x10) para esp0. */
    g_tss.ss0 = 0x10;

    /* Instalar descriptor en la GDT */
    gdt_install_tss((uint32_t)&g_tss, sizeof(g_tss) - 1);

    /* Cargar el selector del TSS en el registro TR (0x28 = entrada 5) */
    __asm__ volatile("ltr %%ax" :: "a"(0x28));
}

void tss_set_esp0(uint32_t esp0)
{
    g_tss.esp0 = esp0;
}
