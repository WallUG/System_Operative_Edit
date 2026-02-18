/*
 * FreeLoader - Simplified Boot Loader
 * Copyright (c) 2024 System_Operative_Edit Project
 * 
 * Este archivo es parte del proyecto System_Operative_Edit
 * Licencia: GPL-3.0
 * 
 * boot_animation.c - Animación de arranque con logo de Universidad de Guayaquil
 * 
 * Implementa una animación de arranque profesional mostrando:
 * - Logo ASCII art de Universidad de Guayaquil (UG)
 * - Barra de progreso animada
 * - Información del sistema
 * - Transiciones suaves con delays
 */

#include "include/freeldr.h"
#include "include/video.h"
#include "include/boot_animation.h"

/*
 * delay - Espera precisa usando BIOS Wait Service
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
static void delay(u32 milliseconds)
{
    unsigned long microseconds = milliseconds * 1000UL;
    unsigned int cx = (microseconds >> 16) & 0xFFFF;  /* High word */
    unsigned int dx = microseconds & 0xFFFF;           /* Low word */
    
    /*
     * INT 15h, AH=86h: Wait (BIOS Wait Service)
     * Este servicio usa el hardware PIT para timing preciso.
     * Es el método estándar usado por ReactOS, GRUB y otros bootloaders.
     */
    __asm__ volatile (
        "int $0x15"
        :                                    /* No output */
        : "a"(0x8600), "c"(cx), "d"(dx)     /* Input: AH=86h, CX:DX=microseconds */
        : "cc"                               /* Clobbers: condition codes */
    );
    
    /*
     * Nota: No hay fallback explícito aquí porque:
     * 1. INT 15h AH=86h está disponible desde 80286+ (prácticamente universal)
     * 2. Si falla, simplemente retorna inmediatamente (no es crítico)
     * 3. El sistema continuará funcionando, solo sin delay
     */
}

/*
 * AnimationInit - Inicializa el sistema de animación
 */
void AnimationInit(void)
{
    /* Por ahora, no necesita inicialización especial */
    /* En el futuro, podría configurar timers o estados */
}

/*
 * AnimationShowLogo - Muestra el logo de Universidad de Guayaquil
 * 
 * Logo simplificado de UG en ASCII art con colores institucionales
 */
void AnimationShowLogo(void)
{
    /* Limpiar pantalla primero */
    VideoClearScreen();
    
    /* Posicionar en la parte superior */
    VideoSetCursor(0, 2);
    
    /* Logo UG en ASCII art - Diseño simplificado pero profesional */
    /* Usando colores institucionales: Azul y Amarillo */
    
    VideoSetColor(MAKE_COLOR(COLOR_LIGHT_CYAN, COLOR_BLACK));
    VideoPutString("                    ========================================\n");
    
    VideoSetColor(MAKE_COLOR(COLOR_YELLOW, COLOR_BLACK));
    VideoPutString("                             _    _    _____  \n");
    VideoPutString("                            | |  | |  / ____| \n");
    VideoPutString("                            | |  | | | |  __  \n");
    VideoPutString("                            | |  | | | | |_ | \n");
    VideoPutString("                            | |__| | | |__| | \n");
    VideoPutString("                             \\____/   \\_____| \n");
    
    VideoSetColor(MAKE_COLOR(COLOR_LIGHT_CYAN, COLOR_BLACK));
    VideoPutString("                    ========================================\n");
    VideoPutString("\n");
    
    VideoSetColor(MAKE_COLOR(COLOR_WHITE, COLOR_BLACK));
    VideoPutString("                       UNIVERSIDAD DE GUAYAQUIL\n");
    
    VideoSetColor(MAKE_COLOR(COLOR_LIGHT_GRAY, COLOR_BLACK));
    VideoPutString("                       System Operative Edit v0.1\n");
    VideoPutString("                       Edicion Universidad de Guayaquil\n");
    
    VideoSetColor(MAKE_COLOR(COLOR_LIGHT_CYAN, COLOR_BLACK));
    VideoPutString("                    ========================================\n");
    VideoPutString("\n");
    
    /* Pequeño delay para que el logo sea visible */
    delay(800);
}

/*
 * AnimationShowProgress - Muestra barra de progreso con animación
 */
void AnimationShowProgress(int step, const char *message)
{
    const int total_steps = 5;
    const int bar_width = 40;
    const int progress_msg_row = 18;
    const int progress_bar_row = 19;
    const int progress_col = 20;
    
    /* Calcular porción llena de la barra */
    int filled = (step * bar_width) / total_steps;
    
    /* Posicionar en parte inferior de la pantalla */
    VideoSetCursor(progress_col, progress_msg_row);
    
    /* Mostrar mensaje de progreso */
    VideoSetColor(MAKE_COLOR(COLOR_LIGHT_CYAN, COLOR_BLACK));
    VideoPutString("  ");
    VideoPutString(message);
    
    /* Limpiar resto de la línea */
    for (int i = 0; i < 60; i++) {
        VideoPutChar(' ');
    }
    
    /* Mostrar barra de progreso */
    VideoSetCursor(progress_col, progress_bar_row);
    VideoSetColor(MAKE_COLOR(COLOR_LIGHT_GRAY, COLOR_BLACK));
    VideoPutString("  [");
    
    /* Parte llena (progreso actual) */
    VideoSetColor(MAKE_COLOR(COLOR_LIGHT_GREEN, COLOR_BLACK));
    for (int i = 0; i < filled; i++) {
        VideoPutChar('=');
    }
    
    /* Indicador de progreso animado */
    if (filled < bar_width) {
        VideoSetColor(MAKE_COLOR(COLOR_YELLOW, COLOR_BLACK));
        VideoPutChar('>');
        filled++;
    }
    
    /* Parte vacía */
    VideoSetColor(MAKE_COLOR(COLOR_DARK_GRAY, COLOR_BLACK));
    for (int i = filled; i < bar_width; i++) {
        VideoPutChar('-');
    }
    
    VideoSetColor(MAKE_COLOR(COLOR_LIGHT_GRAY, COLOR_BLACK));
    VideoPutChar(']');
    
    /* Porcentaje */
    VideoSetColor(MAKE_COLOR(COLOR_LIGHT_CYAN, COLOR_BLACK));
    VideoPutString("  ");
    VideoDecPrint((step * 100) / total_steps);
    VideoPutString("%");
    
    /* Animación de "cargando" con puntos animados */
    delay(150);
}

/*
 * AnimationShowWelcome - Muestra pantalla de bienvenida completa
 */
void AnimationShowWelcome(void)
{
    /* Primero mostrar el logo */
    AnimationShowLogo();
    
    /* Mensaje de inicio */
    VideoSetColor(MAKE_COLOR(COLOR_LIGHT_GRAY, COLOR_BLACK));
    VideoSetCursor(28, 16);
    VideoPutString("Iniciando sistema...");
    
    delay(500);
    
    /* Simular proceso de carga con animación */
    AnimationShowProgress(0, "Inicializando hardware...");
    delay(400);
    
    AnimationShowProgress(1, "Detectando memoria...");
    delay(400);
    
    AnimationShowProgress(2, "Inicializando video...");
    delay(400);
    
    AnimationShowProgress(3, "Configurando disco...");
    delay(400);
    
    AnimationShowProgress(4, "Preparando sistema...");
    delay(400);
    
    /* Mensaje de completitud */
    VideoSetCursor(28, 21);
    VideoSetColor(MAKE_COLOR(COLOR_LIGHT_GREEN, COLOR_BLACK));
    VideoPutString("Listo - Sistema iniciado!");
    delay(600);
}
