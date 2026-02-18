/*
 * vmm.h — Virtual Memory Manager (x86 paginación de 2 niveles)
 *
 * x86 protected mode paging:
 *   CR3 → Page Directory (1024 entradas × 4 bytes = 4KB)
 *   Cada PDE → Page Table (1024 entradas × 4 bytes = 4KB)
 *   Cada PTE → Frame físico de 4KB
 *   Total: 1024 × 1024 × 4KB = 4GB de espacio virtual
 *
 * Layout virtual de cada proceso:
 *   0x00000000 - 0x003FFFFF  →  [NO USAR — null guard]
 *   0x00400000 - 0x7FFEFFFF  →  Código + datos de usuario (Ring 3)
 *   0x7FFF0000 - 0x7FFFFFFF  →  Stack de usuario
 *   0x80000000 - 0x9FFFFFFF  →  [reservado futuro]
 *   0xA0000000 - 0xA000FFFF  →  VGA framebuffer (identity mapped, kernel only)
 *   0xC0000000 - 0xFFFFFFFF  →  Kernel (identity mapped)
 */
#ifndef _VMM_H
#define _VMM_H

#include <types.h>

/* ── Flags de PDE y PTE ─────────────────────────────────────────────────── */
#define PTE_PRESENT     (1 << 0)   /* El frame está mapeado */
#define PTE_WRITABLE    (1 << 1)   /* Lectura/escritura */
#define PTE_USER        (1 << 2)   /* Accesible desde Ring 3 */
#define PTE_ACCESSED    (1 << 5)
#define PTE_DIRTY       (1 << 6)

/* ── Tipos ──────────────────────────────────────────────────────────────── */
typedef uint32_t pde_t;   /* Page Directory Entry */
typedef uint32_t pte_t;   /* Page Table Entry     */

/* Page Directory: 1024 PDEs, alineado a 4KB */
typedef struct {
    pde_t entries[1024];
} __attribute__((aligned(4096))) page_directory_t;

/* Page Table: 1024 PTEs, alineado a 4KB */
typedef struct {
    pte_t entries[1024];
} __attribute__((aligned(4096))) page_table_t;

/* ── API ────────────────────────────────────────────────────────────────── */

/* Crear un nuevo page directory vacío (todo no-presente) */
page_directory_t* vmm_create_directory(void);

/* Mapear virt → phys en un page directory con los flags dados */
void vmm_map_page(page_directory_t* dir,
                  uint32_t virt, uint32_t phys,
                  uint32_t flags);

/* Deshacer el mapeo de una dirección virtual */
void vmm_unmap_page(page_directory_t* dir, uint32_t virt);

/* Activar un page directory (cargar en CR3 + activar paginación si no está) */
void vmm_load_directory(page_directory_t* dir);

/* Inicializar VMM: crear y activar el page directory del kernel */
void vmm_init(void);

/* Obtener el page directory del kernel (para copiar mappings en nuevos procesos) */
page_directory_t* vmm_get_kernel_directory(void);

#endif /* _VMM_H */
