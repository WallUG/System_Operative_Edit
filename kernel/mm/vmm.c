/*
 * vmm.c — Virtual Memory Manager x86
 *
 * Implementa paginación de 2 niveles (page directory + page tables).
 * El kernel usa identity mapping: virt == phys para todo el espacio del kernel.
 * Cada proceso de usuario tiene su propio page directory que incluye
 * los mappings del kernel (para que las syscalls funcionen sin cambiar CR3).
 */
#include "vmm.h"
#include "pmm.h"
#include <types.h>

/* ── Helpers de I/O para CR0/CR3 ─────────────────────────────────────────── */
static inline void write_cr3(uint32_t val)
{
    __asm__ volatile("mov %0, %%cr3" :: "r"(val) : "memory");
}

static inline uint32_t read_cr3(void)
{
    uint32_t val;
    __asm__ volatile("mov %%cr3, %0" : "=r"(val));
    return val;
}

static inline void enable_paging(void)
{
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000001;   /* bit31 = PG, bit0 = PE (ya debería estar) */
    __asm__ volatile("mov %0, %%cr0" :: "r"(cr0) : "memory");
}

static inline void tlb_flush(uint32_t virt)
{
    __asm__ volatile("invlpg (%0)" :: "r"(virt) : "memory");
}

/* ── Estado global ────────────────────────────────────────────────────────── */
static page_directory_t* g_kernel_dir = NULL;

/* ── Internos ─────────────────────────────────────────────────────────────── */

/* Obtener o crear la page table para un PDE */
static page_table_t* get_or_create_table(page_directory_t* dir,
                                          uint32_t pdi, uint32_t flags)
{
    pde_t pde = dir->entries[pdi];

    if (pde & PTE_PRESENT) {
        /* Ya existe — retornar la dirección física como puntero virtual
         * (válido porque usamos identity mapping para el kernel) */
        return (page_table_t*)(pde & ~0xFFF);
    }

    /* Allocar un nuevo frame para la page table */
    uint32_t phys = pmm_alloc_frame();
    if (!phys) return NULL;

    /* Limpiar la nueva page table */
    page_table_t* table = (page_table_t*)phys;
    for (int i = 0; i < 1024; i++)
        table->entries[i] = 0;

    /* Instalar el PDE */
    dir->entries[pdi] = phys | flags | PTE_PRESENT;

    return table;
}

/* ── API pública ──────────────────────────────────────────────────────────── */

page_directory_t* vmm_create_directory(void)
{
    uint32_t phys = pmm_alloc_frame();
    if (!phys) return NULL;

    page_directory_t* dir = (page_directory_t*)phys;

    /* Limpiar todas las entradas */
    for (int i = 0; i < 1024; i++)
        dir->entries[i] = 0;

    /* Copiar las entradas del kernel (mitad alta 0x80000000+) para que
     * las syscalls funcionen sin cambiar CR3 — igual que Windows NT */
    if (g_kernel_dir) {
        for (int i = 512; i < 1024; i++)
            dir->entries[i] = g_kernel_dir->entries[i];
    }

    return dir;
}

void vmm_map_page(page_directory_t* dir,
                  uint32_t virt, uint32_t phys,
                  uint32_t flags)
{
    uint32_t pdi = virt >> 22;          /* bits 31:22 = índice en PD */
    uint32_t pti = (virt >> 12) & 0x3FF; /* bits 21:12 = índice en PT */

    page_table_t* table = get_or_create_table(dir, pdi,
                              flags | PTE_PRESENT | PTE_WRITABLE);
    if (!table) return;

    table->entries[pti] = (phys & ~0xFFF) | flags | PTE_PRESENT;
    tlb_flush(virt);
}

void vmm_unmap_page(page_directory_t* dir, uint32_t virt)
{
    uint32_t pdi = virt >> 22;
    uint32_t pti = (virt >> 12) & 0x3FF;

    pde_t pde = dir->entries[pdi];
    if (!(pde & PTE_PRESENT)) return;

    page_table_t* table = (page_table_t*)(pde & ~0xFFF);
    table->entries[pti] = 0;
    tlb_flush(virt);
}

void vmm_load_directory(page_directory_t* dir)
{
    write_cr3((uint32_t)dir);
}

void vmm_init(void)
{
    uint32_t phys;
    uint32_t addr;

    /* Allocar el page directory del kernel */
    phys = pmm_alloc_frame();
    g_kernel_dir = (page_directory_t*)phys;

    /* Limpiar */
    for (int i = 0; i < 1024; i++)
        g_kernel_dir->entries[i] = 0;

    /*
     * Identity map: 0x00000000 → 0x07FFFFFF (128 MB)
     * Esto cubre: BIOS, kernel image, heap, pilas de kernel.
     * virt == phys para todo lo que ya corre en el kernel.
     */
    for (addr = 0; addr < 0x08000000; addr += PAGE_SIZE) {
        vmm_map_page(g_kernel_dir, addr, addr,
                     PTE_PRESENT | PTE_WRITABLE);
    }

    /*
     * Identity map VGA: 0xA0000 → 0xBFFFF (framebuffer gráfico)
     * Necesario para que el driver VGA siga funcionando.
     * Ya está cubierto por el rango anterior (< 128MB), pero
     * lo dejamos documentado explícitamente.
     */

    /* Activar paginación cargando CR3 y poniendo bit PG en CR0 */
    write_cr3((uint32_t)g_kernel_dir);
    enable_paging();
}

page_directory_t* vmm_get_kernel_directory(void)
{
    return g_kernel_dir;
}
