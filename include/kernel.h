#ifndef _KERNEL_H
#define _KERNEL_H

#include "types.h"

#define KERNEL_VERSION_MAJOR 0
#define KERNEL_VERSION_MINOR 1
#define KERNEL_VERSION_PATCH 0

#define KERNEL_NAME "System Operative Edit"
#define KERNEL_BASE "ReactOS"

/* Kernel panic function */
void kernel_panic(const char* message);

/* Kernel halt */
void kernel_halt(void);

#endif /* _KERNEL_H */
