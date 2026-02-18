/*
 * scheduler.h — Round-Robin Scheduler al estilo Windows NT
 *
 * ARQUITECTURA (igual que NT Kernel Dispatcher):
 *
 *   IRQ0 (timer 18.2Hz)
 *       │
 *       ▼
 *   irq0_timer_handler (idt.c, assembly)
 *       │  llama a scheduler_tick() con el cpu_context_t del thread interrumpido
 *       │
 *       ▼
 *   scheduler_tick()
 *       │  decrementa quantum del thread actual
 *       │  si quantum == 0 → context_switch()
 *       │
 *       ▼
 *   context_switch()
 *       │  guarda el contexto del thread actual en saved_context
 *       │  elige el siguiente thread READY (round-robin)
 *       │  carga el page directory del nuevo proceso (CR3)
 *       │  restaura el contexto del nuevo thread
 *       │  IRET → nuevo thread continúa
 *       │
 *       ▼
 *   Nuevo thread ejecutándose en su propio stack/page directory
 *
 * QUANTUM por defecto: 5 ticks (~275ms a 18.2Hz)
 * El idle process (PID 0) siempre tiene quantum=1 y corre si no hay nadie más
 */
#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include <types.h>
#include "proc/process.h"

/* Número de ticks de timer por quantum (5 ticks ≈ 275ms a 18.2Hz) */
#define SCHEDULER_QUANTUM   5

/* Inicializar el scheduler — debe llamarse DESPUÉS de proc_init() */
void scheduler_init(void);

/*
 * Llamado desde el handler de IRQ0 (timer) en cada tick.
 * ctx: puntero al cpu_context_t del thread interrumpido (en su kernel stack).
 * Retorna el puntero al cpu_context_t del próximo thread a ejecutar.
 * Si retorna el mismo ctx, no hubo cambio de contexto.
 */
cpu_context_t* scheduler_tick(cpu_context_t* ctx);

/* Añadir un thread a la cola del scheduler */
void scheduler_add_thread(thread_t* t);

/* Remover un thread de la cola (cuando muere o se bloquea) */
void scheduler_remove_thread(thread_t* t);

/* Forzar un yield del thread actual (cede el CPU voluntariamente) */
void scheduler_yield(void);

/* Estadísticas — número de context switches totales */
uint32_t scheduler_get_switches(void);

#endif /* _SCHEDULER_H */
