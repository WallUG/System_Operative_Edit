/* syscall.h - números de syscall compartidos entre kernel y librería de usuario */
#ifndef _SYSCALL_H
#define _SYSCALL_H

/* Deben coincidir con los valores en libsys.h */
#define SYS_EXIT        0x00
#define SYS_YIELD       0x01
#define SYS_DRAW_PIXEL  0x02
#define SYS_FILL_RECT   0x03
#define SYS_DRAW_STRING 0x04
#define SYS_GET_TICK    0x05
#define SYS_GET_MOUSE        0x06   /* DEPRECATED, solo Ring 0 */
#define SYS_GET_MOUSE_STATE  0x07   /* Ring 3 seguro: copia por valor */
#define SYS_GET_PIXEL        0x08   /* Leer pixel del shadow buffer (x, y) -> color 0-15 */
#define SYS_DEBUG            0x09   /* imprimir cadena en serial para depuración */
#define SYS_DUMP_VRAM        0x0A   /* debug: dump first part of VGA memory */

#define SYSCALL_ERR      ((uint32_t)-1)

#endif /* _SYSCALL_H */
