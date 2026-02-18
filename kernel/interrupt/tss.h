/*
 * tss.h — Task State Segment (TSS) para Ring 3 → Ring 0
 *
 * PARA QUÉ SIRVE EL TSS EN v0.3:
 *
 * Cuando un proceso Ring 3 ejecuta INT 0x30 (syscall) o recibe una
 * interrupción (IRQ0 timer), la CPU necesita cambiar al stack de Ring 0.
 * ¿De dónde saca ese stack? Del TSS.
 *
 * El campo TSS.ESP0 contiene el ESP del kernel que la CPU usará cuando
 * haga la transición Ring 3 → Ring 0. Antes de ejecutar cualquier
 * proceso de usuario, el kernel actualiza TSS.ESP0 con el tope del
 * kernel stack de ese proceso.
 *
 * Sin TSS: al intentar INT desde Ring 3, #GP (General Protection Fault).
 *
 * LAYOUT del TSS que necesitamos (mínimo):
 *   offset 0  : link  (selector del TSS anterior — 0 si no hay)
 *   offset 4  : esp0  ← el que más importa: stack kernel para Ring 3
 *   offset 8  : ss0   ← selector de segmento de datos del kernel (0x10)
 *   ... (resto de campos, los ponemos en 0)
 *
 * El GDT necesita un descriptor apuntando al TSS (entrada 5, selector 0x28).
 * La instrucción LTR (Load Task Register) activa el TSS.
 */
#ifndef _TSS_H
#define _TSS_H

#include <types.h>

/* ── Estructura TSS x86 (campos mínimos usados) ──────────────────────── */
typedef struct {
    uint32_t link;       /* 0x00 - TSS anterior (0 = ninguno) */
    uint32_t esp0;       /* 0x04 - ESP kernel para Ring 3 → Ring 0 ← CLAVE */
    uint32_t ss0;        /* 0x08 - SS kernel (0x10) */
    uint32_t esp1;       /* 0x0C */
    uint32_t ss1;        /* 0x10 */
    uint32_t esp2;       /* 0x14 */
    uint32_t ss2;        /* 0x18 */
    uint32_t cr3;        /* 0x1C */
    uint32_t eip;        /* 0x20 */
    uint32_t eflags;     /* 0x24 */
    uint32_t eax;        /* 0x28 */
    uint32_t ecx;        /* 0x2C */
    uint32_t edx;        /* 0x30 */
    uint32_t ebx;        /* 0x34 */
    uint32_t esp;        /* 0x38 */
    uint32_t ebp;        /* 0x3C */
    uint32_t esi;        /* 0x40 */
    uint32_t edi;        /* 0x44 */
    uint32_t es;         /* 0x48 */
    uint32_t cs;         /* 0x4C */
    uint32_t ss;         /* 0x50 */
    uint32_t ds;         /* 0x54 */
    uint32_t fs;         /* 0x58 */
    uint32_t gs;         /* 0x5C */
    uint32_t ldt;        /* 0x60 */
    uint16_t trap;       /* 0x64 */
    uint16_t iomap_base; /* 0x66 - offset del IO Permission Bitmap */
} __attribute__((packed)) tss_t;

/* ── API ─────────────────────────────────────────────────────────────── */

/* Inicializar TSS y añadir su descriptor al GDT (entrada 5, selector 0x28) */
void tss_init(void);

/*
 * Actualizar ESP0 en el TSS antes de hacer context switch a un thread
 * de usuario. La CPU usará este valor cuando el proceso haga una syscall
 * o reciba una interrupción.
 *
 * Llamar con: kernel_stack_top del thread que va a ejecutarse.
 */
void tss_set_kernel_stack(uint32_t esp0);

#endif /* _TSS_H */
