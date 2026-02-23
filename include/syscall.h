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

/* nuevos syscalls para el servicio GUI y cursor */
#define SYS_GET_MOUSE_EVENT   0x0B   /* obtener próximo evento de mouse */
#define SYS_SET_CURSOR_POS    0x0C   /* (x,y) */
#define SYS_HIDE_CURSOR       0x0D
#define SYS_SHOW_CURSOR       0x0E

#define SYS_GUI_DRAW_DESKTOP      0x0F
#define SYS_GUI_DRAW_TASKBAR      0x10
#define SYS_GUI_DRAW_WINDOW       0x11   /* pointer a GUI_WINDOW */
#define SYS_GUI_DRAW_WINDOW_TEXT  0x12   /* a=pointer, b=rx, c=ry, d=pointer, e=fg */
#define SYS_GUI_DRAW_BUTTON       0x13   /* a=x,b=y,c=w,d=h,e=pressed, pointer label in esi */

#define SYSCALL_ERR      ((uint32_t)-1)

#endif /* _SYSCALL_H */
