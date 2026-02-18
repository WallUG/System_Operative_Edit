/*
 * pmm.c — Physical Memory Manager (bitmap allocator)
 *
 * Divide la RAM en frames de 4096 bytes.
 * Usa un bitmap estático de 2048 bytes → cubre hasta 64MB (16384 frames).
 * Cada bit representa un frame: 0=libre, 1=usado.
 */
#include "pmm.h"
#include <types.h>

/* ── Configuración ────────────────────────────────────────────────────────── */
/* Número máximo de frames que manejamos (64MB / 4KB = 16384) */
#define MAX_FRAMES      16384
/* El bitmap ocupa MAX_FRAMES / 8 bytes = 2048 bytes */
#define BITMAP_SIZE     (MAX_FRAMES / 8)

/* ── Estado interno ───────────────────────────────────────────────────────── */
static uint8_t  pmm_bitmap[BITMAP_SIZE];   /* 0=libre, 1=usado */
static uint32_t pmm_total_frames = 0;
static uint32_t pmm_used  = 0;

/* Dirección física del primer frame manejado */
#define BASE_ADDR   PMM_FREE_START

/* ── Helpers de bitmap ────────────────────────────────────────────────────── */
static inline void bitmap_set(uint32_t frame)
{
    pmm_bitmap[frame / 8] |= (1 << (frame % 8));
}

static inline void bitmap_clear(uint32_t frame)
{
    pmm_bitmap[frame / 8] &= ~(uint8_t)(1 << (frame % 8));
}

static inline int bitmap_test(uint32_t frame)
{
    return (pmm_bitmap[frame / 8] >> (frame % 8)) & 1;
}

/* ── API pública ──────────────────────────────────────────────────────────── */

void pmm_init(uint32_t mem_upper_kb)
{
    uint32_t i;
    uint32_t mem_end;

    /* Marcar todo como usado por defecto (seguro) */
    for (i = 0; i < BITMAP_SIZE; i++)
        pmm_bitmap[i] = 0xFF;

    /* Calcular extremo de RAM disponible */
    mem_end = (mem_upper_kb + 1024) * 1024;   /* mem_upper en KB desde 1MB */
    if (mem_end > PMM_FREE_END)
        mem_end = PMM_FREE_END;

    /* Liberar frames en el rango PMM_FREE_START..mem_end */
    {
        uint32_t start_frame = (PMM_FREE_START - BASE_ADDR) / PAGE_SIZE;
        uint32_t end_frame   = (mem_end        - BASE_ADDR) / PAGE_SIZE;

        if (end_frame > MAX_FRAMES)
            end_frame = MAX_FRAMES;

        pmm_total_frames = end_frame - start_frame;

        for (i = start_frame; i < end_frame; i++)
            bitmap_clear(i);

        pmm_used = 0;
    }
}

uint32_t pmm_alloc_frame(void)
{
    uint32_t i, j;

    for (i = 0; i < BITMAP_SIZE; i++) {
        if (pmm_bitmap[i] == 0xFF) continue;   /* byte lleno, siguiente */
        for (j = 0; j < 8; j++) {
            uint32_t frame = i * 8 + j;
            if (!bitmap_test(frame)) {
                bitmap_set(frame);
                pmm_used++;
                return BASE_ADDR + frame * PAGE_SIZE;
            }
        }
    }
    return 0;   /* sin memoria */
}

void pmm_free_frame(uint32_t addr)
{
    if (addr < BASE_ADDR) return;
    uint32_t frame = (addr - BASE_ADDR) / PAGE_SIZE;
    if (frame >= MAX_FRAMES) return;
    if (bitmap_test(frame)) {
        bitmap_clear(frame);
        pmm_used--;
    }
}

uint32_t pmm_free_frames(void) { return pmm_total_frames - pmm_used; }
uint32_t pmm_used_frames(void) { return pmm_used; }
