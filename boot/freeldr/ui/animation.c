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

/* Delay simple usando bucle ocupado */
static void animation_delay(unsigned int milliseconds)
{
    /* Aproximadamente 1ms por cada 50000 iteraciones en CPU moderna típica */
    volatile unsigned int count = milliseconds * 50000;
    while (count--) {
        __asm__ volatile ("nop");
    }
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
