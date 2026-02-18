/*
 * boot_splash.h — Animación de arranque del kernel
 *
 * Llamar KernelShowBootSplash() desde kernel_main() ANTES de
 * gdt_init() o cualquier mensaje en modo texto.
 *
 * Si el BIOS no soporta cambio de modo de video, la función retorna
 * inmediatamente sin modificar nada — el kernel sigue su camino normal.
 */
#ifndef _BOOT_SPLASH_H
#define _BOOT_SPLASH_H

/*
 * KernelShowBootSplash — muestra la animación de boot en modo VGA 13h.
 *
 * Secuencia:
 *   1. Cambia a modo gráfico 320×200×256 via INT 0x10
 *   2. Muestra logo UG + barra de progreso (5 fases, ~2.5 seg total)
 *   3. Restaura modo texto 0x03
 *
 * Seguro de llamar sin ninguna inicialización previa del kernel.
 */
void KernelShowBootSplash(void);

#endif /* _BOOT_SPLASH_H */
