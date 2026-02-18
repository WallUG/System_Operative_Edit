/*
 * Animation - Graphics Boot Animation
 * Copyright (c) 2024 System_Operative_Edit Project
 * Based on ReactOS UI architecture
 * 
 * Este archivo implementa la animación de boot gráfica con logo UG
 * 
 * Licencia: GPL-3.0
 */

#include "bootvid.h"
#include "bootlogo.h"

/* Define NULL if not already defined */
#ifndef NULL
#define NULL ((void*)0)
#endif

/*
 * animation_delay - Espera precisa usando BIOS Wait Service
 * 
 * Usa INT 15h AH=86h (BIOS Wait Service) para timing preciso basado en hardware.
 * Este método usa el PIT (Programmable Interval Timer) del sistema, proporcionando
 * delays consistentes independientemente de la velocidad del CPU.
 * 
 * @milliseconds: Tiempo a esperar en milisegundos
 * 
 * BIOS INT 15h, AH=86h: Wait
 * Entrada:
 *   AH = 86h
 *   CX:DX = Intervalo en microsegundos (32-bit: CX=high word, DX=low word)
 * 
 * Ventajas sobre busy-wait loop:
 * - Timing preciso independiente de CPU
 * - Usa hardware PIT timer
 * - Funciona consistentemente en emuladores (VirtualBox, QEMU)
 * - Soluciona problemas de sincronización VGA
 * 
 * Soluciona: Barras de colores aleatorias durante boot en emuladores
 * debido a timing insuficiente en inicialización de modo VGA
 */
static void animation_delay(unsigned int milliseconds)
{
    unsigned long microseconds;
    unsigned int cx, dx;
    
    /*
     * Limitar a 65535 ms (~65 segundos) para prevenir overflow
     * BIOS INT 15h AH=86h típicamente soporta hasta ~1 segundo,
     * pero algunos implementan hasta ~65 segundos (CX:DX máximo)
     * Nota: 65535 * 1000 = 65,535,000 que cabe en unsigned long 32-bit (max: 4,294,967,295)
     */
    if (milliseconds > 65535) {
        milliseconds = 65535;
    }
    
    microseconds = milliseconds * 1000UL;  /* Safe: max 65,535,000 < 2^32 */
    cx = (microseconds >> 16) & 0xFFFF;  /* High word */
    dx = microseconds & 0xFFFF;           /* Low word */
    
    /*
     * INT 15h, AH=86h: Wait (BIOS Wait Service)
     * Este servicio usa el hardware PIT para timing preciso.
     * Es el método estándar usado por ReactOS, GRUB y otros bootloaders.
     */
    __asm__ volatile (
        "int $0x15"
        :                                    /* No output */
        : "a"(0x8600), "c"(cx), "d"(dx)     /* Input: AH=86h, CX:DX=microseconds */
        : "cc", "memory"                     /* Clobbers: flags, memory barrier */
    );
    
    /*
     * Nota: No hay fallback explícito aquí porque:
     * 1. INT 15h AH=86h está disponible desde 80286+ (prácticamente universal)
     * 2. Si falla, simplemente retorna inmediatamente (no es crítico)
     * 3. El sistema continuará funcionando, solo sin delay
     * 4. "memory" en clobber list previene reordenamiento de código
     */
}

/*
 * AnimationGraphicsInit - Inicializa el sistema de animación gráfica
 * 
 * Retorna: 1 si el modo gráfico se inicializó correctamente, 0 si falla
 */
int AnimationGraphicsInit(void)
{
    /* Intentar inicializar modo gráfico VGA */
    if (!BootVidInitialize()) {
        return 0; /* Falló, usar fallback a texto */
    }
    
    /* Configurar paleta de colores institucionales */
    BootVidInitializePalette();
    
    /* Limpiar pantalla a negro */
    BootVidClearScreen(COLOR_BLACK);
    
    return 1;
}

/*
 * AnimationGraphicsShowLogo - Muestra el logo de Universidad de Guayaquil con fade in
 */
void AnimationGraphicsShowLogo(void)
{
    if (!BootVidIsActive()) {
        return;
    }
    
    /* Limpiar pantalla */
    BootVidClearScreen(COLOR_BLACK);
    
    /* Renderizar logo UG (usando logo embebido) */
    BootLogoRender(NULL, 0);
    
    /* Efecto de fade in */
    BootVidFadeScreen(1, 10); /* 10 pasos de fade in */
    
    /* Mantener logo visible */
    animation_delay(1000);
}

/*
 * AnimationGraphicsShowBranding - Muestra información del sistema debajo del logo
 */
void AnimationGraphicsShowBranding(void)
{
    if (!BootVidIsActive()) {
        return;
    }
    
    /* Dibujar textos debajo del logo */
    /* Nota: La función DrawText actual es placeholder, dibuja bloques */
    /* En una implementación completa, usaría un bitmap font */
    
    /* Por ahora, solo dibujamos rectángulos decorativos */
    int y_base = 140;
    
    /* Línea decorativa superior */
    BootVidDrawRect(60, y_base, 200, 2, COLOR_YELLOW);
    
    /* Rectángulos representando texto (placeholder) */
    /* "System Operative Edit v0.1" */
    BootVidDrawRect(70, y_base + 10, 180, 8, COLOR_WHITE);
    
    /* "Universidad de Guayaquil" */
    BootVidDrawRect(80, y_base + 25, 160, 8, COLOR_YELLOW);
    
    /* Línea decorativa inferior */
    BootVidDrawRect(60, y_base + 40, 200, 2, COLOR_YELLOW);
    
    animation_delay(500);
}

/*
 * AnimationGraphicsShowProgress - Muestra barra de progreso con animación
 * 
 * @step: Paso actual (0-4)
 * @total_steps: Total de pasos
 * @message: Mensaje a mostrar (no usado en modo gráfico actual)
 */
void AnimationGraphicsShowProgress(int step, int total_steps, const char *message __attribute__((unused)))
{
    if (!BootVidIsActive()) {
        return;
    }
    
    /* Calcular porcentaje */
    int progress = (step * 100) / total_steps;
    
    /* Dibujar barra de progreso en la parte inferior */
    BootLogoDrawProgressBar(progress, 170);
    
    /* Delay para animación */
    animation_delay(400);
}

/*
 * AnimationGraphicsComplete - Muestra animación de completitud
 */
void AnimationGraphicsComplete(void)
{
    if (!BootVidIsActive()) {
        return;
    }
    
    /* Actualizar barra a 100% */
    BootLogoDrawProgressBar(100, 170);
    
    animation_delay(600);
    
    /* Efecto de fade out */
    BootVidFadeScreen(0, 10); /* 10 pasos de fade out */
}

/*
 * AnimationGraphicsCleanup - Limpia y restaura modo de video
 */
void AnimationGraphicsCleanup(void)
{
    if (BootVidIsActive()) {
        BootVidResetDisplay();
    }
}

/*
 * AnimationGraphicsShowWelcome - Secuencia completa de animación
 * 
 * Esta es la función principal que orquesta toda la animación:
 * 1. Pantalla negra
 * 2. Fade in del logo UG
 * 3. Branding
 * 4. Barra de progreso animada (5 etapas)
 * 5. Fade out
 */
void AnimationGraphicsShowWelcome(void)
{
    if (!BootVidIsActive()) {
        return;
    }
    
    /* Paso 1: Pantalla negra inicial */
    BootVidClearScreen(COLOR_BLACK);
    animation_delay(500);
    
    /* Paso 2: Fade in del logo */
    AnimationGraphicsShowLogo();
    
    /* Paso 3: Mostrar branding */
    AnimationGraphicsShowBranding();
    
    /* Paso 4: Barra de progreso animada */
    const int total_steps = 5;
    
    AnimationGraphicsShowProgress(0, total_steps, "Inicializando hardware...");
    AnimationGraphicsShowProgress(1, total_steps, "Detectando memoria...");
    AnimationGraphicsShowProgress(2, total_steps, "Inicializando video...");
    AnimationGraphicsShowProgress(3, total_steps, "Configurando disco...");
    AnimationGraphicsShowProgress(4, total_steps, "Preparando sistema...");
    AnimationGraphicsShowProgress(5, total_steps, "Completado");
    
    /* Paso 5: Animación de completitud y fade out */
    AnimationGraphicsComplete();
}
