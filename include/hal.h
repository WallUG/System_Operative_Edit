#ifndef _HAL_H
#define _HAL_H

#include "types.h"

/* Hardware Abstraction Layer interface */

/* Initialize HAL */
void hal_init(void);

/* I/O port operations */
uint8_t inb(uint16_t port);
void outb(uint16_t port, uint8_t value);

uint16_t inw(uint16_t port);
void outw(uint16_t port, uint16_t value);

uint32_t inl(uint16_t port);
void outl(uint16_t port, uint32_t value);

/* CPU operations */
void cpu_halt(void);
void cpu_disable_interrupts(void);
void cpu_enable_interrupts(void);

#endif /* _HAL_H */
