/*
 * process.h — Process Control Block y API de gestión de procesos
 *
 * Modelo inspirado en Windows NT:
 *   - Cada proceso tiene su propio page directory (espacio de direcciones)
 *   - Cada proceso tiene al menos un thread
 *   - El scheduler trabaja sobre threads, no sobre procesos
 *   - El kernel corre en Ring 0, los procesos de usuario en Ring 3
 *
 * Estados de un thread:
 *   READY   → en la cola del scheduler, esperando CPU
 *   RUNNING → actualmente ejecutándose
 *   BLOCKED → esperando un evento (I/O, sleep, etc.)
 *   DEAD    → terminó, recursos pendientes de liberar
 */
#ifndef _PROCESS_H
#define _PROCESS_H

#include <types.h>
#include "../mm/vmm.h"

/* ── Límites ────────────────────────────────────────────────────────────── */
#define MAX_PROCESSES   16
#define MAX_THREADS     32
#define KERNEL_STACK_SIZE  8192    /* 8KB por thread de kernel */
#define USER_STACK_TOP     0x7FFF0000
#define USER_STACK_SIZE    0x10000   /* 64KB de stack de usuario */

/* ── Niveles de privilegio ──────────────────────────────────────────────── */
#define PRIVILEGE_KERNEL  0   /* Ring 0 */
#define PRIVILEGE_USER    3   /* Ring 3 */

/* ── Estado del thread ──────────────────────────────────────────────────── */
typedef enum {
    THREAD_READY   = 0,
    THREAD_RUNNING = 1,
    THREAD_BLOCKED = 2,
    THREAD_DEAD    = 3
} thread_state_t;

/* ── Contexto de CPU guardado en un cambio de contexto ─────────────────── */
/*
 * Al interrumpir un thread, la CPU empuja automáticamente en su stack:
 *   [ESP]    EIP     (instruction pointer)
 *   [ESP+4]  CS      (code segment)
 *   [ESP+8]  EFLAGS
 *   [ESP+12] ESP_usr (solo si viene de Ring 3)
 *   [ESP+16] SS_usr  (solo si viene de Ring 3)
 *
 * Nosotros empujamos el resto con PUSHA + segmentos:
 */
typedef struct {
    /* Empujados por nuestra rutina (orden inverso al stack) */
    uint32_t gs, fs, es, ds;
    /* Empujados por PUSHA */
    uint32_t edi, esi, ebp, esp_dummy;
    uint32_t ebx, edx, ecx, eax;
    /* Número de interrupción e error code (empujados por el stub) */
    uint32_t int_no, err_code;
    /* Empujados automáticamente por la CPU */
    uint32_t eip, cs, eflags;
    /* Solo si viene de Ring 3 */
    uint32_t user_esp, user_ss;
} cpu_context_t;

/* ── Thread Control Block ───────────────────────────────────────────────── */
typedef struct _thread {
    uint32_t        tid;
    uint32_t        pid;            /* proceso al que pertenece */
    thread_state_t  state;
    uint32_t        privilege;      /* PRIVILEGE_KERNEL o PRIVILEGE_USER */

    /* Stack del kernel para este thread (siempre presente) */
    uint32_t        kernel_stack_top;   /* ESP inicial en el stack del kernel */
    uint32_t        kernel_stack_base;  /* dirección física del inicio del bloque */

    /* Stack de usuario (solo para threads de usuario) */
    uint32_t        user_stack_top;

    /* Puntero al stack frame guardado durante el context switch */
    cpu_context_t*  saved_context;

    /* Quantum restante (decrementado por IRQ0) */
    int32_t         quantum;

    /* Lista enlazada simple para la cola del scheduler */
    struct _thread* next;
} thread_t;

/* ── Process Control Block ──────────────────────────────────────────────── */
typedef struct _process {
    uint32_t            pid;
    char                name[32];
    uint32_t            privilege;
    page_directory_t*   page_dir;       /* espacio de direcciones propio */
    thread_t*           main_thread;
    uint32_t            active;         /* 1 = activo */
} process_t;

/* ── API ────────────────────────────────────────────────────────────────── */

/* Inicializar el subsistema de procesos (crea proceso idle del kernel) */
void proc_init(void);

/*
 * Crear un proceso de kernel (Ring 0).
 * entry_point: función C a ejecutar.
 * El proceso se añade a la cola del scheduler en estado READY.
 */
process_t* proc_create_kernel(const char* name,
                               void (*entry_point)(void));

/*
 * Crear el proceso GUI en Ring 3 (v0.4+).
 *
 * Versión simplificada de proc_create_user: toma directamente un
 * entry_point del kernel, crea un page directory con el kernel mapeado
 * como PTE_USER, y construye el frame de IRET para Ring 3.
 *
 * Úsala en lugar de proc_create_kernel para el gui_server.
 */
process_t* proc_create_user_gui(const char* name,
                                 void (*entry_point)(void));

/*
 * Crear un proceso de usuario (Ring 3) genérico.
 * code_phys:  dirección física donde está el código del proceso.
 * code_size:  tamaño del código en bytes.
 * entry_virt: dirección virtual del punto de entrada (dentro del proceso).
 *
 * El VMM mapea el código en el espacio de usuario y crea un stack en Ring 3.
 */
process_t* proc_create_user(const char* name,
                             uint32_t code_phys,
                             uint32_t code_size,
                             uint32_t entry_virt);

/* Terminar el proceso actual (llamado desde syscall o explícitamente) */
void proc_exit(uint32_t exit_code);

/* Obtener el thread actualmente en ejecución */
thread_t* proc_current_thread(void);

/* Obtener el proceso del thread actual */
process_t* proc_current_process(void);

/* Buscar proceso por PID (usado por el scheduler para cambiar CR3) */
process_t* proc_get_process_by_pid(uint32_t pid);

/* Setter usado por el scheduler */
void proc_set_current_thread(thread_t* t);

/* Getter de la tabla de threads (usado por el scheduler) */
thread_t* proc_get_thread_table(void);

#endif /* _PROCESS_H */
