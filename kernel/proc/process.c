/*
 * process.c — Implementación del gestor de procesos
 */
#include "process.h"
#include "../mm/pmm.h"
#include "../mm/vmm.h"
#include <types.h>

/* Forward declaration para evitar dependencia circular con scheduler.h */
extern void scheduler_add_thread(thread_t* t);

/* memset desde lib */
extern void* memset(void*, int, size_t);

/* ── Tablas globales ────────────────────────────────────────────────────── */
static process_t g_processes[MAX_PROCESSES];
static thread_t  g_threads[MAX_THREADS];

static uint32_t  g_next_pid = 1;
static uint32_t  g_next_tid = 1;

/* Thread actualmente en ejecución (actualizado por el scheduler) */
static thread_t* g_current_thread = NULL;

/* ── Helpers internos ───────────────────────────────────────────────────── */

static process_t* alloc_process(void)
{
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (!g_processes[i].active) {
            memset(&g_processes[i], 0, sizeof(process_t));
            g_processes[i].active = 1;
            g_processes[i].pid    = g_next_pid++;
            return &g_processes[i];
        }
    }
    return NULL;
}

static thread_t* alloc_thread(void)
{
    for (int i = 0; i < MAX_THREADS; i++) {
        if (g_threads[i].state == THREAD_DEAD &&
            g_threads[i].tid == 0) {
            memset(&g_threads[i], 0, sizeof(thread_t));
            g_threads[i].tid = g_next_tid++;
            return &g_threads[i];
        }
    }
    /* Buscar slot nunca usado (tid == 0) */
    for (int i = 0; i < MAX_THREADS; i++) {
        if (g_threads[i].tid == 0) {
            memset(&g_threads[i], 0, sizeof(thread_t));
            g_threads[i].tid = g_next_tid++;
            return &g_threads[i];
        }
    }
    return NULL;
}

/*
 * Preparar el stack del kernel de un thread de kernel.
 *
 * Cuando el scheduler haga el primer context switch a este thread,
 * ejecutará un IRET usando los valores que pusimos en el stack.
 * Necesitamos simular la estructura que deja una interrupción:
 *
 *   [stack_top - 4]  EFLAGS  (IF=1 para que el thread arranque con irqs)
 *   [stack_top - 8]  CS      (0x08 = kernel code segment)
 *   [stack_top - 12] EIP     (entry_point)
 *
 * El scheduler también espera un frame de cpu_context_t encima de eso
 * (los registros guardados). Los ponemos todos en 0.
 */
static cpu_context_t* setup_kernel_stack(uint32_t stack_top,
                                          void (*entry_point)(void))
{
    /*
     * El IRQ0 handler restaura el contexto en este orden exacto:
     *
     *   pop ds, pop es, pop fs, pop gs    <- tope del stack (addr mas baja)
     *   popa                               <- 8 registros generales
     *   add esp, 8                         <- descarta int_no + err_code
     *   iret                               <- consume eip, cs, eflags
     *
     * Por tanto debemos construir el stack de ABAJO HACIA ARRIBA
     * (decrementando esp), de modo que al leerlo de arriba hacia abajo
     * quede: gs, fs, es, ds | edi,esi,ebp,esp_d,ebx,edx,ecx,eax | err,int | eip,cs,efl
     *
     * Al decrementar esp primero empujamos lo que quedara al FONDO
     * (lo ultimo en desapilarse = iret frame), luego lo que queda arriba.
     */
    uint32_t* esp = (uint32_t*)stack_top;

    /* 1. iret frame: CPU lo consume ultimo (fondo del frame) */
    *(--esp) = 0x00000202;            /* EFLAGS: IF=1, bit1=1 siempre */
    *(--esp) = 0x08;                  /* CS: kernel code selector */
    *(--esp) = (uint32_t)entry_point; /* EIP: donde arranca el thread */

    /* 2. int_no y err_code ficticios (add esp,8 los descarta) */
    *(--esp) = 0;   /* err_code */
    *(--esp) = 0;   /* int_no   */

    /* 3. PUSHA: popa los restaura en orden edi,esi,ebp,esp,ebx,edx,ecx,eax
     *    Empujamos en orden inverso: eax primero (queda al fondo del bloque pusha) */
    *(--esp) = 0;   /* eax */
    *(--esp) = 0;   /* ecx */
    *(--esp) = 0;   /* edx */
    *(--esp) = 0;   /* ebx */
    *(--esp) = 0;   /* esp_dummy (popa lo ignora) */
    *(--esp) = 0;   /* ebp */
    *(--esp) = 0;   /* esi */
    *(--esp) = 0;   /* edi */

    /* 4. Segmentos: pop gs/fs/es/ds los restaura en ese orden
     *    Empujamos en orden inverso: gs queda al tope (menor dir) */
    *(--esp) = 0x10;   /* gs */
    *(--esp) = 0x10;   /* fs */
    *(--esp) = 0x10;   /* es */
    *(--esp) = 0x10;   /* ds - tope del stack, primero en desapilarse */

    return (cpu_context_t*)esp;
}

/*
 * Preparar el stack del kernel de un thread de usuario.
 * En Ring 3→0 la CPU sí empuja user_esp y user_ss.
 */
static cpu_context_t* setup_user_stack(uint32_t kstack_top,
                                        uint32_t entry_virt,
                                        uint32_t user_esp)
{
    uint32_t* esp = (uint32_t*)kstack_top;

    /* Ring 3 IRET frame: user_ss, user_esp, eflags, cs, eip */
    *(--esp) = 0x23;           /* SS usuario (selector 0x20 | RPL 3) */
    *(--esp) = user_esp;       /* ESP de usuario */
    *(--esp) = 0x00000202;     /* EFLAGS: IF=1 */
    *(--esp) = 0x1B;           /* CS usuario (selector 0x18 | RPL 3) */
    *(--esp) = entry_virt;     /* EIP de entrada */

    *(--esp) = 0; *(--esp) = 0;   /* int_no, err_code */

    /* PUSHA */
    *(--esp) = 0; *(--esp) = 0; *(--esp) = 0; *(--esp) = 0;
    *(--esp) = 0; *(--esp) = 0; *(--esp) = 0; *(--esp) = 0;

    /* Segmentos de usuario */
    *(--esp) = 0x23;   /* ds */
    *(--esp) = 0x23;   /* es */
    *(--esp) = 0x23;   /* fs */
    *(--esp) = 0x23;   /* gs */

    return (cpu_context_t*)esp;
}

/* ── Proceso idle del kernel ────────────────────────────────────────────── */
static void kernel_idle(void)
{
    /* El proceso idle solo cede la CPU continuamente.
     * Corre cuando no hay ningún otro thread READY. */
    while (1) {
        __asm__ volatile("sti; hlt");
    }
}

/* ── API pública ────────────────────────────────────────────────────────── */

void proc_init(void)
{
    /* Limpiar tablas */
    memset(g_processes, 0, sizeof(g_processes));
    memset(g_threads,   0, sizeof(g_threads));

    /* Crear el proceso idle (PID 0 — siempre listo) */
    process_t* idle = alloc_process();
    idle->pid       = 0;   /* sobreescribir — idle es siempre PID 0 */
    g_next_pid      = 1;   /* resetear para el primer proceso real */

    /* Copiar nombre */
    idle->name[0] = 'i'; idle->name[1] = 'd';
    idle->name[2] = 'l'; idle->name[3] = 'e';
    idle->name[4] = '\0';

    idle->privilege = PRIVILEGE_KERNEL;
    idle->page_dir  = vmm_get_kernel_directory();

    /* Thread del idle */
    thread_t* t = alloc_thread();
    t->pid       = idle->pid;
    t->privilege = PRIVILEGE_KERNEL;
    t->quantum   = 1;
    t->state     = THREAD_READY;

    /* Stack del kernel para el idle */
    uint32_t kstack = pmm_alloc_frame();
    t->kernel_stack_base = kstack;
    t->kernel_stack_top  = kstack + PAGE_SIZE;

    t->saved_context = setup_kernel_stack(t->kernel_stack_top, kernel_idle);

    idle->main_thread = t;
    g_current_thread  = t;   /* arrancamos "en" el idle */
}

process_t* proc_create_kernel(const char* name, void (*entry_point)(void))
{
    process_t* proc = alloc_process();
    if (!proc) return NULL;

    /* Nombre */
    int i;
    for (i = 0; i < 31 && name[i]; i++) proc->name[i] = name[i];
    proc->name[i] = '\0';

    proc->privilege = PRIVILEGE_KERNEL;
    proc->page_dir  = vmm_get_kernel_directory();

    /* Thread principal */
    thread_t* t = alloc_thread();
    if (!t) { proc->active = 0; return NULL; }

    t->pid       = proc->pid;
    t->privilege = PRIVILEGE_KERNEL;
    t->quantum   = 5;
    t->state     = THREAD_READY;

    uint32_t kstack = pmm_alloc_frame();
    t->kernel_stack_base = kstack;
    t->kernel_stack_top  = kstack + PAGE_SIZE;
    t->saved_context = setup_kernel_stack(t->kernel_stack_top, entry_point);

    proc->main_thread = t;

    /* Registrar el thread en el scheduler automaticamente */
    scheduler_add_thread(t);

    return proc;
}

process_t* proc_create_user(const char* name,
                             uint32_t code_phys,
                             uint32_t code_size,
                             uint32_t entry_virt)
{
    /* DEBUG: mostrar parámetros de la imagen de usuario
     * (también los emitimos por serial para poder analizarlos en logs) */
    extern void screen_writeln(const char*);
    extern void screen_write(const char*);
    extern void serial_puts(const char*);
    /* debug: dump start/address/size as hex (also send digits to serial) */
    screen_write("user start=0x");
    serial_puts("user start=0x");
    {
        uint32_t v = code_phys;
        for (int i = 28; i >= 0; i -= 4) {
            char c = "0123456789ABCDEF"[(v >> i) & 0xF];
            char s[2] = {c,0};
            screen_write(s);
            serial_puts(s);
        }
        screen_writeln("");
        serial_puts("\r\n");
    }
    screen_write("entry=0x");
    serial_puts("entry=0x");
    {
        uint32_t v = entry_virt;
        for (int i = 28; i >= 0; i -= 4) {
            char c = "0123456789ABCDEF"[(v >> i) & 0xF];
            char s[2] = {c,0};
            screen_write(s);
            serial_puts(s);
        }
        screen_writeln("");
        serial_puts("\r\n");
    }
    screen_write("size=0x");
    serial_puts("size=0x");
    {
        uint32_t v = code_size;
        for (int i = 28; i >= 0; i -= 4) {
            char c = "0123456789ABCDEF"[(v >> i) & 0xF];
            char s[2] = {c,0};
            screen_write(s);
            serial_puts(s);
        }
        screen_writeln("");
        serial_puts("\r\n");
    }

    process_t* proc = alloc_process();
    if (!proc) return NULL;

    int i;
    for (i = 0; i < 31 && name[i]; i++) proc->name[i] = name[i];
    proc->name[i] = '\0';

    proc->privilege = PRIVILEGE_USER;

    /* Crear page directory propio para este proceso */
    proc->page_dir = vmm_create_directory();
    if (!proc->page_dir) { proc->active = 0; return NULL; }

    /* Mapear el código del proceso en el espacio de usuario */
    uint32_t virt = entry_virt & ~(PAGE_SIZE - 1);   /* alinear */
    uint32_t phys = code_phys  & ~(PAGE_SIZE - 1);
    uint32_t pages = (code_size + PAGE_SIZE - 1) / PAGE_SIZE;

    for (uint32_t p = 0; p < pages; p++) {
        /* mapeo de pagina para procesos de usuario
         * notas:
         * - Antes marcabamos solo PTE_USER, que deja las paginas como
         *   read-only. Esto causaba fallos al escribir en .user.data
         *   (p.ej. el buffer del cursor) porque el slot no era escribible.
         *   Para la prueba de GUI es más sencillo mapear TODO el espacio
         *   de usuario como escribible; podemos refinar si queremos
         *   volver a proteger el codigo.
         */
        vmm_map_page(proc->page_dir,
                     virt + p * PAGE_SIZE,
                     phys + p * PAGE_SIZE,
                     PTE_PRESENT | PTE_USER | PTE_WRITABLE);
    }

    /* Mapear stack de usuario */
    uint32_t stack_virt = USER_STACK_TOP - USER_STACK_SIZE;
    uint32_t stack_pages = USER_STACK_SIZE / PAGE_SIZE;

    for (uint32_t p = 0; p < stack_pages; p++) {
        uint32_t frame = pmm_alloc_frame();
        if (!frame) { proc->active = 0; return NULL; }
        vmm_map_page(proc->page_dir,
                     stack_virt + p * PAGE_SIZE,
                     frame,
                     PTE_PRESENT | PTE_WRITABLE | PTE_USER);
    }

    /* Thread principal */
    thread_t* t = alloc_thread();
    if (!t) { proc->active = 0; return NULL; }

    t->pid            = proc->pid;
    t->privilege      = PRIVILEGE_USER;
    t->quantum        = 5;
    t->state          = THREAD_READY;
    t->user_stack_top = USER_STACK_TOP;

    /* Stack del KERNEL para este thread (para manejar syscalls/irqs) */
    uint32_t kstack = pmm_alloc_frame();
    t->kernel_stack_base = kstack;
    t->kernel_stack_top  = kstack + PAGE_SIZE;
    t->saved_context = setup_user_stack(t->kernel_stack_top,
                                         entry_virt,
                                         USER_STACK_TOP - 4);

    proc->main_thread = t;

    /* Fix: algunas funciones en .user (como user_entry) usan PIC y
     * llaman a __x86.get_pc_thunk.* situado en la sección .text del
     * kernel. Esas páginas no están accesibles a Ring 3, provocando
     * page faults al entrar. Mapeamos explícitamente la página que
     * contiene el thunk para que el proceso pueda ejecutarlo. */
    {
        const uint32_t thunk_addr = 0x00106ccb;
        uint32_t page = thunk_addr & ~(PAGE_SIZE - 1);
        vmm_map_page(proc->page_dir, page, page,
                     PTE_PRESENT | PTE_USER /* no escribible */);
    }

    /* Registrar el thread en la cola del scheduler para que pueda correr */
    scheduler_add_thread(t);

    return proc;
}

void proc_exit(uint32_t exit_code)
{
    (void)exit_code;
    if (g_current_thread) {
        g_current_thread->state = THREAD_DEAD;
    }
    /* El scheduler elegirá el siguiente thread en el próximo tick */
    __asm__ volatile("sti; hlt");
    while(1);
}

thread_t* proc_current_thread(void)  { return g_current_thread; }

process_t* proc_get_process_by_pid(uint32_t pid)
{
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (g_processes[i].active && g_processes[i].pid == pid)
            return &g_processes[i];
    }
    return NULL;
}

process_t* proc_current_process(void)
{
    if (!g_current_thread) return NULL;
    uint32_t pid = g_current_thread->pid;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (g_processes[i].active && g_processes[i].pid == pid)
            return &g_processes[i];
    }
    return NULL;
}

/* Setter usado por el scheduler */
void proc_set_current_thread(thread_t* t)
{
    g_current_thread = t;
}

/* Getter de la tabla de threads (usado por el scheduler) */
thread_t* proc_get_thread_table(void)
{
    return g_threads;
}
