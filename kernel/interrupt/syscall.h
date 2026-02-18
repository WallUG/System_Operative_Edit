/*
 * syscall.h — Interfaz de syscalls (INT 0x30)
 *
 * ARQUITECTURA (al estilo Windows NT):
 *
 *   Proceso Ring 3                    Kernel Ring 0
 *   ──────────────                    ─────────────
 *   syscall(SYS_DRAW_PIXEL, ...)
 *     │  mov eax, número
 *     │  mov ebx, arg1
 *     │  ...
 *     └─ INT 0x30
 *                    ──────────────────►
 *                                       syscall_handler (asm, idt.c)
 *                                         guarda contexto
 *                                         llama syscall_dispatch(ctx)
 *                                         restaura contexto
 *                                       IRET → Ring 3
 *   retorno en EAX ◄──────────────────
 *
 * CONVENCIÓN DE LLAMADA:
 *   EAX = número de syscall (SYS_*)
 *   EBX = argumento 1
 *   ECX = argumento 2
 *   EDX = argumento 3
 *   ESI = argumento 4
 *   EDI = argumento 5
 *   Retorno en EAX
 *
 * TABLA DE SYSCALLS v0.3:
 *   0x00  SYS_EXIT         — terminar el proceso actual
 *   0x01  SYS_YIELD        — ceder CPU voluntariamente
 *   0x02  SYS_DRAW_PIXEL   — dibujar un pixel (x, y, color)
 *   0x03  SYS_FILL_RECT    — rellenar rectangulo (x, y, w, h, color)
 *   0x04  SYS_DRAW_STRING  — dibujar texto (x, y, str_phys, fg, bg)
 *   0x05  SYS_GET_TICK     — obtener tick counter del kernel
 *   0x06  SYS_GET_MOUSE    — obtener estado del mouse (ptr a struct)
 */
#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <types.h>
#include "proc/process.h"

/* ── Números de syscall ──────────────────────────────────────────────── */
#define SYS_EXIT        0x00
#define SYS_YIELD       0x01
#define SYS_DRAW_PIXEL  0x02
#define SYS_FILL_RECT   0x03
#define SYS_DRAW_STRING 0x04
#define SYS_GET_TICK    0x05
#define SYS_GET_MOUSE   0x06

#define SYSCALL_COUNT   7

/* Valor de retorno de error genérico */
#define SYSCALL_ERR   ((uint32_t)(-1))
#define SYSCALL_OK    0

/* ── API del kernel ──────────────────────────────────────────────────── */

/*
 * Inicializar el subsistema de syscalls:
 *   - Registra INT 0x30 en la IDT con DPL=3 (accesible desde Ring 3)
 */
void syscall_init(void);

/*
 * Dispatcher de syscalls — llamado desde el handler INT 0x30.
 * ctx: contexto completo del proceso interrumpido.
 * Lee EAX (número) y EBX/ECX/EDX/ESI/EDI (args) del contexto.
 * Escribe el retorno en ctx->eax.
 */
void syscall_dispatch(cpu_context_t* ctx);

#endif /* _SYSCALL_H */
