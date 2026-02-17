/*
 * PROJECT:     System_Operative_Edit
 * LICENSE:     GPL-3.0
 * PURPOSE:     HAL Display Support - Header
 * COPYRIGHT:   Adapted from ReactOS (GPL-3.0)
 *              Original: ReactOS Project (hal/halx86/generic/display.c)
 *              Adaptation: Universidad de Guayaquil
 */

#ifndef _HAL_DISPLAY_H
#define _HAL_DISPLAY_H

#include <drivers/ddk/wdm.h>

/**
 * HalInitializeDisplay - Initialize HAL display subsystem
 * 
 * Returns: STATUS_SUCCESS or error code
 */
NTSTATUS HalInitializeDisplay(VOID);

/**
 * HalDisplayString - Display a string using HAL
 * @String: String to display
 */
VOID HalDisplayString(const char *String);

/**
 * HalQueryDisplayParameters - Query display parameters
 * @Width: Receives display width
 * @Height: Receives display height
 * @Depth: Receives color depth
 * 
 * Returns: STATUS_SUCCESS or error code
 */
NTSTATUS HalQueryDisplayParameters(
    OUT PULONG Width,
    OUT PULONG Height,
    OUT PULONG Depth
);

#endif /* _HAL_DISPLAY_H */
