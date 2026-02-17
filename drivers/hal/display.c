/*
 * PROJECT:     System_Operative_Edit
 * LICENSE:     GPL-3.0
 * PURPOSE:     HAL Display Support
 * COPYRIGHT:   Adapted from ReactOS (GPL-3.0)
 *              Original: ReactOS Project (hal/halx86/generic/display.c)
 *              Adaptation: Universidad de Guayaquil
 */

#include "display.h"
#include "video/vga/vga.h"

/* Display initialization state */
static BOOLEAN g_DisplayInitialized = FALSE;

/**
 * HalInitializeDisplay - Initialize HAL display subsystem
 * 
 * Returns: STATUS_SUCCESS or error code
 */
NTSTATUS HalInitializeDisplay(VOID)
{
    if (g_DisplayInitialized) {
        return STATUS_SUCCESS;
    }
    
    /* Display initialization is handled by the VGA driver */
    g_DisplayInitialized = TRUE;
    
    return STATUS_SUCCESS;
}

/**
 * HalDisplayString - Display a string using HAL
 * @String: String to display
 */
VOID HalDisplayString(const char *String)
{
    /* Simple display function - could be extended to show text in graphics mode */
    if (!String) {
        return;
    }
    
    /* For now, this is a stub. In a full implementation, this would:
     * - Display text in graphics mode using a bitmap font
     * - Handle newlines and scrolling
     * - Maintain a text buffer
     */
}

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
)
{
    PDEVICE_OBJECT VgaDevice;
    PVGA_DEVICE_EXTENSION DevExt;
    
    if (!Width || !Height || !Depth) {
        return STATUS_INVALID_PARAMETER;
    }
    
    /* Get VGA device */
    VgaDevice = VgaGetDeviceObject();
    if (!VgaDevice) {
        return STATUS_DEVICE_DOES_NOT_EXIST;
    }
    
    DevExt = (PVGA_DEVICE_EXTENSION)VgaDevice->DeviceExtension;
    if (!DevExt) {
        return STATUS_INVALID_PARAMETER;
    }
    
    /* Return display parameters */
    *Width = DevExt->ScreenWidth;
    *Height = DevExt->ScreenHeight;
    *Depth = DevExt->BitsPerPixel;
    
    return STATUS_SUCCESS;
}
