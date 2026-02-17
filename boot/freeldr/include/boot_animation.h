/*
 * FreeLoader - Simplified Boot Loader
 * Copyright (c) 2024 System_Operative_Edit Project
 * 
 * Este archivo es parte del proyecto System_Operative_Edit
 * Licencia: GPL-3.0
 * 
 * boot_animation.h - Animación de arranque con logo de Universidad de Guayaquil
 */

#ifndef _BOOT_ANIMATION_H
#define _BOOT_ANIMATION_H

#include "freeldr.h"

/*
 * AnimationInit - Inicializa el sistema de animación
 * 
 * Prepara el sistema para mostrar la animación de arranque
 */
void AnimationInit(void);

/*
 * AnimationShowLogo - Muestra el logo de Universidad de Guayaquil
 * 
 * Despliega el logo UG en modo texto usando ASCII art con colores
 */
void AnimationShowLogo(void);

/*
 * AnimationShowProgress - Muestra barra de progreso con animación
 * @step: Paso actual (0-4)
 * @message: Mensaje a mostrar
 * 
 * Muestra una barra de progreso animada durante el proceso de arranque
 */
void AnimationShowProgress(int step, const char *message);

/*
 * AnimationShowWelcome - Muestra pantalla de bienvenida completa
 * 
 * Combina logo, información del sistema y animación de inicio
 */
void AnimationShowWelcome(void);

#endif /* _BOOT_ANIMATION_H */
