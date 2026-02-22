#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <types.h>
#include "../proc/process.h"  /* proc_exit */
#include "../proc/scheduler.h" /* scheduler_yield */
#include "../../include/syscall.h" /* números de syscall */

/* Entrada de la interrupción de syscall (vector 0x30).
 * El stub en assembly hace el salvado/restauración de regs y
 * llama a syscall_dispatch().
 */
void syscall_entry(void);

/* Dispatcher interno, recibe los 6 argumentos extraídos por el stub. */
uint32_t syscall_dispatch(uint32_t num, uint32_t a, uint32_t b,
                          uint32_t c, uint32_t d, uint32_t e);

#endif /* _SYSCALL_H */
