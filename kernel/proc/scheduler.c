/*
 * scheduler.c — Round-Robin Scheduler
 *
 * Implementa el dispatcher de Windows NT simplificado:
 *
 *   Cola de threads READY (lista circular enlazada)
 *   ┌──────────────────────────────────────────┐
 *   │  idle → gui_thread → kernel_thread → ... │
 *   │    ↑_________________________________↑   │
 *   └──────────────────────────────────────────┘
 *
 * En cada tick del timer (IRQ0) (ahora configurado a 100 Hz):
 *   1. Decrementar quantum del thread actual
 *   2. Si quantum > 0: retornar el mismo contexto (sin cambio)
 *   3. Si quantum = 0: salvar contexto actual, avanzar al siguiente READY,
 *      cargar su CR3 si cambió de proceso, restaurar su contexto → IRET
 *
 * El context switch real ocurre en assembly (en idt.c):
 *   el handler de IRQ0 guarda ESP en saved_context del thread actual,
 *   luego scheduler_tick() retorna el nuevo ESP, y el handler hace IRET
 *   desde el nuevo stack → el nuevo thread continúa como si nada.
 *
 * IMPORTANT: scheduler_tick() se llama desde el handler de interrupción,
 * por lo tanto corre con interrupciones DESHABILITADAS (IF=0).
 * No llamar a funciones que necesiten interrupciones aquí.
 */

#include "scheduler.h"
#include "process.h"
#include "../mm/vmm.h"
#include <types.h>

/* ── Estado del scheduler ─────────────────────────────────────────────── */

/* Cola circular de threads listos para ejecutar. Se mantiene también un
 * puntero "tail" para inserciones O(1) y, más adelante, podríamos convertirla
 * en lista doble para remociones rápidas. */
static thread_t*  sched_queue_head = NULL;   /* cabeza de la cola */
static thread_t*  sched_queue_tail = NULL;   /* último elemento (apunta a head) */
static thread_t*  sched_current    = NULL;   /* thread ejecutándose ahora */
static uint32_t   sched_switches   = 0;      /* contador de context switches */

/* ── Helpers de cola ──────────────────────────────────────────────────── */

/*
 * Insertar thread al final de la cola circular.
 * La cola es una lista circular simple: last->next = head.
 */
static void queue_insert(thread_t* t)
{
    if (!t) return;

    if (!sched_queue_head) {
        /* Cola vacía: el thread apunta a sí mismo */
        sched_queue_head = t;
        sched_queue_tail = t;
        t->next = t;
    } else {
        /* Inserción al final aprovechando "tail" para O(1" ) */
        t->next = sched_queue_head;
        sched_queue_tail->next = t;
        sched_queue_tail = t;
    }
}

/*
 * Remover un thread de la cola circular.
 * Si el thread a remover es sched_queue_head, avanzar la cabeza.
 */
static void queue_remove(thread_t* t)
{
    if (!sched_queue_head || !t) return;

    /* Caso: solo hay un elemento */
    if (sched_queue_head->next == sched_queue_head &&
        sched_queue_head == t) {
        sched_queue_head = NULL;
        sched_queue_tail = NULL;
        t->next = NULL;
        return;
    }

    /* Buscar nodo anterior a t (necesario con lista simple) */
    thread_t* prev = sched_queue_head;
    while (prev->next != t && prev->next != sched_queue_head) {
        prev = prev->next;
    }

    if (prev->next != t) return;  /* t no está en la cola */

    prev->next = t->next;

    if (sched_queue_head == t) {
        sched_queue_head = t->next;
    }
    if (sched_queue_tail == t) {
        /* tail debe moverse al anterior que acabamos de encontrar */
        sched_queue_tail = prev;
    }

    t->next = NULL;
}

/*
 * Elegir el siguiente thread READY en la cola.
 * Empieza desde el sucesor del actual y da una vuelta completa.
 * Si nadie está READY, retorna el idle (siempre está en la cola y siempre READY).
 */
static thread_t* queue_next_ready(thread_t* current)
{
    if (!sched_queue_head) return NULL;

    /* Punto de partida: el sucesor del thread actual */
    thread_t* start = current ? current->next : sched_queue_head;
    if (!start) start = sched_queue_head;

    thread_t* candidate = start;
    do {
        if (candidate->state == THREAD_READY) {
            return candidate;
        }
        candidate = candidate->next;
    } while (candidate != start);

    /* Nadie READY — retornar el mismo si sigue READY (p.ej. idle) */
    if (current && current->state == THREAD_READY) return current;

    return NULL;
}

/* ── Cambio de CR3 ────────────────────────────────────────────────────── */
static inline void load_cr3(uint32_t phys_dir)
{
    __asm__ volatile("mov %0, %%cr3" :: "r"(phys_dir) : "memory");
}

/* ── Debug: contador de context switches en VGA texto ─────────────────── */
static void debug_write_switches(uint32_t count)
{
    volatile uint16_t* vga = (volatile uint16_t*)0xB8000;
    const uint8_t COLOR = 0x5F;
    char buf[6];
    int i;
    uint32_t n = count;

    buf[5] = '\0';
    for (i = 4; i >= 0; i--) {
        buf[i] = '0' + (n % 10);
        n /= 10;
    }
    vga[72] = (uint16_t)'S' | ((uint16_t)COLOR << 8);
    vga[73] = (uint16_t)'W' | ((uint16_t)COLOR << 8);
    vga[74] = (uint16_t)':' | ((uint16_t)COLOR << 8);
    for (i = 0; i < 5; i++)
        vga[75 + i] = (uint16_t)(unsigned char)buf[i] | ((uint16_t)COLOR << 8);
}

/* ── API pública ──────────────────────────────────────────────────────── */

void scheduler_init(void)
{
    /*
     * En este punto proc_create_kernel() ya llamo scheduler_add_thread()
     * para el gui_server, por lo que sched_queue_head apunta a ese thread.
     * Solo necesitamos añadir el idle a la cola existente y marcar
     * el thread actual (kernel_main actuando como idle).
     *
     * NO reiniciar sched_queue_head — eso romperia el enlace al gui_server.
     */
    thread_t* idle = proc_current_thread();   /* thread idle creado por proc_init */
    if (!idle) return;

    /* Insertar el idle en la cola (que ya contiene el gui_server) */
    queue_insert(idle);

    /* El idle empieza como RUNNING porque es el thread que esta ejecutando
     * kernel_main en este momento. El scheduler lo cambiara a READY en
     * el primer tick y elegira el gui_server. */
    idle->state   = THREAD_RUNNING;
    idle->quantum = SCHEDULER_QUANTUM;
    sched_current = idle;
}

void scheduler_add_thread(thread_t* t)
{
    if (!t) return;
    t->state = THREAD_READY;
    queue_insert(t);
}

void scheduler_remove_thread(thread_t* t)
{
    if (!t) return;
    queue_remove(t);
}

/*
 * scheduler_tick — corazón del dispatcher.
 *
 * Parámetro ctx: ESP del thread interrumpido, apuntando a su cpu_context_t
 *                en su kernel stack.
 * Retorno:       ESP del próximo thread (puede ser el mismo si no hay cambio).
 *
 * Llamado desde irq0_timer_handler en idt.c con IF=0.
 */
cpu_context_t* scheduler_tick(cpu_context_t* ctx)
{
    /*
     * Invariante de seguridad: si algo falla, devolvemos 'ctx' (el
     * contexto actual) para que el sistema siga vivo aunque no cambie.
     */
    if (!sched_current) return ctx;

    /* 1. Guardar contexto del thread actual */
    sched_current->saved_context = ctx;

    /* 2. Marcar como READY para que pueda ser re-elegido */
    if (sched_current->state == THREAD_RUNNING) {
        sched_current->state = THREAD_READY;
    }

    /* 3. Decrementar quantum */
    if (sched_current->quantum > 0)
        sched_current->quantum--;

    /* 4. Si le queda quantum, seguir con el mismo thread */
    if (sched_current->quantum > 0 && sched_current->state == THREAD_READY) {
        sched_current->state = THREAD_RUNNING;
        return ctx;
    }

    /* 5. Quantum agotado: buscar el siguiente thread READY */
    thread_t* next = queue_next_ready(sched_current);

    /* Si no hay otro thread disponible, continuar con el actual */
    if (!next || next == sched_current) {
        sched_current->quantum = SCHEDULER_QUANTUM;
        sched_current->state   = THREAD_RUNNING;
        return ctx;
    }

    /* 6. --- CONTEXT SWITCH --- */
    sched_switches++;
    debug_write_switches(sched_switches);   /* DEBUG: contador visible en VGA texto */

    /* Resetear quantum del siguiente */
    next->quantum = SCHEDULER_QUANTUM;
    next->state   = THREAD_RUNNING;

    /* 7. Cambiar CR3 si el nuevo thread pertenece a un proceso diferente */
    if (next->pid != sched_current->pid) {
        extern process_t* proc_get_process_by_pid(uint32_t pid);
        process_t* np = proc_get_process_by_pid(next->pid);
        if (np && np->page_dir)
            load_cr3((uint32_t)np->page_dir);
    }

    /* 8. Actualizar puntero al thread actual */
    proc_set_current_thread(next);
    sched_current = next;

    /* 9. SEGURIDAD: si el nuevo thread nunca ha corrido, saved_context
     *    apunta al frame inicial que setup_kernel_stack() construyo.
     *    Ese frame ya tiene eip=entry_point y eflags correcto.
     *    Si por alguna razon saved_context es NULL, devolver ctx actual
     *    para no corromper el stack. */
    if (!next->saved_context) {
        /* No deberia ocurrir con setup_kernel_stack() correcto,
         * pero si ocurre, revertir el switch para no crashear. */
        sched_current = sched_current;   /* no-op, ya asignado */
        return ctx;
    }

    /* 10. Retornar el ESP del nuevo thread — el handler hace IRET desde el */
    return next->saved_context;
}

void scheduler_yield(void)
{
    /* Forzar un yield: poner quantum a 0 y disparar una interrupción soft.
     * El timer lo recogerá en el próximo tick. Para un yield inmediato
     * usamos INT 0x20 (nuestra IRQ0 remapeada). */
    if (sched_current) {
        sched_current->quantum = 0;
    }
    /* Trigger IRQ0 via INT — fuerza el context switch ahora mismo */
    __asm__ volatile("int $0x20");
}

uint32_t scheduler_get_switches(void)
{
    return sched_switches;
}
