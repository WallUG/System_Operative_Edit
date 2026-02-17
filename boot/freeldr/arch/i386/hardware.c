/*
 * Hardware - i386 Hardware Initialization
 * Copyright (c) 2024 System_Operative_Edit Project
 * Based on ReactOS hardware.c
 * 
 * Este archivo maneja la inicialización de hardware para i386,
 * incluyendo el modo gráfico VGA para la animación de boot
 * 
 * Licencia: GPL-3.0
 */

#include "bootvid.h"

/* Estado del hardware */
static int hardware_initialized = 0;
static int graphics_available = 0;

/*
 * HwInitialize - Inicializa el hardware básico
 * 
 * Retorna: 1 si exitoso, 0 si falla
 */
int HwInitialize(void)
{
    if (hardware_initialized) {
        return 1;
    }
    
    /* Aquí se inicializarían otros componentes de hardware */
    /* Por ahora, solo marcamos como inicializado */
    
    hardware_initialized = 1;
    return 1;
}

/*
 * HwInitBootGraphics - Inicializa el modo gráfico para el boot
 * 
 * Intenta configurar el modo gráfico VGA para la animación.
 * Si falla, el sistema puede continuar en modo texto.
 * 
 * Retorna: 1 si el modo gráfico está disponible, 0 si no
 */
int HwInitBootGraphics(void)
{
    /* Asegurar que el hardware básico está inicializado */
    if (!hardware_initialized) {
        if (!HwInitialize()) {
            return 0;
        }
    }
    
    /* Intentar inicializar BootVid (modo VGA gráfico) */
    if (BootVidInitialize()) {
        graphics_available = 1;
        return 1;
    }
    
    /* Falló el modo gráfico, continuar en modo texto */
    graphics_available = 0;
    return 0;
}

/*
 * HwIsGraphicsAvailable - Verifica si el modo gráfico está disponible
 */
int HwIsGraphicsAvailable(void)
{
    return graphics_available;
}

/*
 * HwResetGraphics - Restaura el modo de video original
 */
void HwResetGraphics(void)
{
    if (graphics_available) {
        BootVidResetDisplay();
        graphics_available = 0;
    }
}
