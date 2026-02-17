/*
 * Hardware - i386 Hardware Initialization Header
 * Copyright (c) 2024 System_Operative_Edit Project
 * 
 * Licencia: GPL-3.0
 */

#ifndef _HARDWARE_H_
#define _HARDWARE_H_

/*
 * Inicializa el hardware básico del sistema
 * Retorna: 1 si exitoso, 0 si falla
 */
int HwInitialize(void);

/*
 * Inicializa el modo gráfico para el boot
 * Retorna: 1 si el modo gráfico está disponible, 0 si no
 */
int HwInitBootGraphics(void);

/*
 * Verifica si el modo gráfico está disponible
 * Retorna: 1 si disponible, 0 si no
 */
int HwIsGraphicsAvailable(void);

/*
 * Restaura el modo de video original
 */
void HwResetGraphics(void);

#endif /* _HARDWARE_H_ */
