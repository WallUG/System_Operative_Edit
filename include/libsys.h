/*
 * libsys.h — Librería de syscalls para procesos de usuario
 *
 * Esta librería provee wrappers en C sobre las llamadas INT 0x30.
 * Es lo equivalente a win32.dll / ntdll.dll en Windows NT:
 * el proceso no llama directamente al kernel — llama a estas funciones
 * que manejan la convención de llamada y el INT 0x30.
 *
 * CONVENCIÓN:
 *   EAX = número de syscall
 *   EBX = arg1, ECX = arg2, EDX = arg3, ESI = arg4, EDI = arg5
 *   Retorno en EAX
 *
 * En v0.3 el gui_server corre en Ring 0 con paginación, por lo que
 * estas llamadas funcionan tanto desde Ring 0 como Ring 3. En v0.4
 * cuando esté en Ring 3 real, serán el único canal al kernel.
 */
#ifndef _LIBSYS_H
#define _LIBSYS_H

#include <types.h>

/* Números de syscall (deben coincidir con syscall.h del kernel) */
#define SYS_EXIT        0x00
#define SYS_YIELD       0x01
#define SYS_DRAW_PIXEL  0x02
#define SYS_FILL_RECT   0x03
#define SYS_DRAW_STRING 0x04
#define SYS_GET_TICK    0x05
#define SYS_GET_MOUSE        0x06   /* DEPRECATED — solo Ring 0 */
#define SYS_GET_MOUSE_STATE  0x07   /* Ring 3 seguro: copia por valor */

/* Estado del mouse (misma estructura que el kernel) */
typedef struct {
    int x;
    int y;
    int buttons;
} SYS_MOUSE;

/* ── Macros inline para cada syscall ─────────────────────────────────── */

static inline uint32_t sys_exit(uint32_t code)
{
    uint32_t ret;
    __asm__ volatile(
        "int $0x30"
        : "=a"(ret)
        : "a"(SYS_EXIT), "b"(code)
        : "memory"
    );
    return ret;
}

static inline uint32_t sys_yield(void)
{
    uint32_t ret;
    __asm__ volatile(
        "int $0x30"
        : "=a"(ret)
        : "a"(SYS_YIELD)
        : "memory"
    );
    return ret;
}

static inline uint32_t sys_draw_pixel(int x, int y, uint8_t color)
{
    uint32_t ret;
    __asm__ volatile(
        "int $0x30"
        : "=a"(ret)
        : "a"(SYS_DRAW_PIXEL), "b"(x), "c"(y), "d"(color)
        : "memory"
    );
    return ret;
}

static inline uint32_t sys_fill_rect(int x, int y, int w, int h, uint8_t color)
{
    uint32_t ret;
    __asm__ volatile(
        "int $0x30"
        : "=a"(ret)
        : "a"(SYS_FILL_RECT), "b"(x), "c"(y), "d"(w), "S"(h), "D"(color)
        : "memory"
    );
    return ret;
}

static inline uint32_t sys_draw_string(int x, int y, const char* s,
                                        uint8_t fg, uint8_t bg)
{
    uint32_t ret;
    __asm__ volatile(
        "int $0x30"
        : "=a"(ret)
        : "a"(SYS_DRAW_STRING), "b"(x), "c"(y), "d"(s), "S"(fg), "D"(bg)
        : "memory"
    );
    return ret;
}

static inline uint32_t sys_get_tick(void)
{
    uint32_t ret;
    __asm__ volatile(
        "int $0x30"
        : "=a"(ret)
        : "a"(SYS_GET_TICK)
        : "memory"
    );
    return ret;
}

/* DEPRECATED: devuelve puntero al kernel — seguro solo en Ring 0 */
static inline SYS_MOUSE* sys_get_mouse(void)
{
    uint32_t ret;
    __asm__ volatile(
        "int $0x30"
        : "=a"(ret)
        : "a"(SYS_GET_MOUSE)
        : "memory"
    );
    return (SYS_MOUSE*)ret;
}

/*
 * sys_get_mouse_state — versión segura para Ring 3.
 *
 * Copia el MOUSE_STATE por valor a un buffer local del proceso.
 * El kernel valida que el puntero destino está en el espacio de usuario
 * antes de escribir, por lo que es seguro desde Ring 3.
 *
 * Uso:
 *   SYS_MOUSE ms;
 *   if (sys_get_mouse_state(&ms) == 0) { ... usa ms.x, ms.y ... }
 */
static inline uint32_t sys_get_mouse_state(SYS_MOUSE* out)
{
    uint32_t ret;
    __asm__ volatile(
        "int $0x30"
        : "=a"(ret)
        : "a"(SYS_GET_MOUSE_STATE), "b"(out)
        : "memory"
    );
    return ret;
}

#endif /* _LIBSYS_H */
