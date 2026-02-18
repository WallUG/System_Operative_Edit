/*
 * pmm.h — Physical Memory Manager
 *
 * Gestiona frames de 4KB usando un bitmap.
 * Un bit = 1 frame de 4096 bytes.
 * Bit 0 = frame libre, Bit 1 = frame usado.
 *
 * Layout de memoria física:
 *   0x00000 - 0x0FFFF  → Reservado (BIOS, IVT, VGA text)
 *   0x10000 - 0x9FFFF  → Reservado (EBDA, ROM)
 *   0xA0000 - 0xBFFFF  → VGA framebuffer — NO tocar
 *   0x100000- 0x1FFFFF → Kernel image (cargado por GRUB)
 *   0x200000- ...      → Frames libres para procesos y heap
 */
#ifndef _PMM_H
#define _PMM_H

#include <types.h>

#define PAGE_SIZE       4096
#define PAGE_ALIGN(a)   (((a) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))

/* Rango de frames libres (fisica) — empieza en 8MB para no pisar kernel */
#define PMM_FREE_START  0x00800000   /* 8 MB */
#define PMM_FREE_END    0x04000000   /* 64 MB — ajustar segun RAM real */

/* Inicializar el PMM con los datos de memoria de multiboot */
void pmm_init(uint32_t mem_upper_kb);

/* Allocar un frame fisico — retorna direccion fisica o 0 si no hay */
uint32_t pmm_alloc_frame(void);

/* Liberar un frame fisico */
void pmm_free_frame(uint32_t addr);

/* Estadisticas */
uint32_t pmm_free_frames(void);
uint32_t pmm_used_frames(void);

#endif /* _PMM_H */
