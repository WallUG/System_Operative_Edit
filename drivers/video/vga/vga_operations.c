/*
 * PROJECT:     System_Operative_Edit
 * LICENSE:     GPL-3.0
 * PURPOSE:     VGA Drawing Operations and Demos
 * COPYRIGHT:   Adapted from ReactOS VGA Driver (GPL-3.0)
 *              Original: ReactOS Project (win32ss/drivers/displays/vga/objects)
 *              Adaptation: Universidad de Guayaquil
 */

#include "vga.h"

/**
 * VgaDrawRect - Draw a rectangle outline
 * @x: X coordinate of top-left corner
 * @y: Y coordinate of top-left corner
 * @width: Width of rectangle
 * @height: Height of rectangle
 * @Color: Border color (0-15)
 */
VOID VgaDrawRect(INT x, INT y, INT width, INT height, UCHAR Color)
{
    /* Top and bottom borders */
    VgaDrawLine(x, y, x + width - 1, y, Color);
    VgaDrawLine(x, y + height - 1, x + width - 1, y + height - 1, Color);
    
    /* Left and right borders */
    VgaDrawLine(x, y, x, y + height - 1, Color);
    VgaDrawLine(x + width - 1, y, x + width - 1, y + height - 1, Color);
}

/**
 * VgaDrawDemo - Draw a demo pattern to test VGA functionality
 */
VOID VgaDrawDemo(VOID)
{
    PDEVICE_OBJECT VgaDevice;
    PVGA_DEVICE_EXTENSION DevExt;
    
    VgaDevice = VgaGetDeviceObject();
    if (!VgaDevice) {
        return;
    }
    
    DevExt = (PVGA_DEVICE_EXTENSION)VgaDevice->DeviceExtension;
    if (!DevExt || !DevExt->GraphicsMode) {
        return;
    }
    
    /* Clear screen to black */
    VgaClearScreen(VGA_COLOR_BLACK);
    
    /* Draw color bars across the top */
    for (int i = 0; i < 16; i++) {
        VgaFillRect(i * 40, 10, 40, 30, (UCHAR)i);
    }
    
    /* Draw colored rectangles */
    VgaDrawRect(50, 60, 100, 80, VGA_COLOR_WHITE);
    VgaFillRect(55, 65, 90, 70, VGA_COLOR_BLUE);
    
    VgaDrawRect(200, 60, 100, 80, VGA_COLOR_WHITE);
    VgaFillRect(205, 65, 90, 70, VGA_COLOR_RED);
    
    VgaDrawRect(350, 60, 100, 80, VGA_COLOR_WHITE);
    VgaFillRect(355, 65, 90, 70, VGA_COLOR_GREEN);
    
    /* Draw lines */
    VgaDrawLine(50, 200, 590, 200, VGA_COLOR_YELLOW);
    VgaDrawLine(50, 250, 590, 250, VGA_COLOR_CYAN);
    VgaDrawLine(50, 300, 590, 300, VGA_COLOR_LIGHT_MAGENTA);
    
    /* Draw diagonal lines */
    VgaDrawLine(100, 350, 200, 450, VGA_COLOR_WHITE);
    VgaDrawLine(250, 350, 350, 450, VGA_COLOR_LIGHT_CYAN);
    VgaDrawLine(400, 350, 500, 450, VGA_COLOR_LIGHT_RED);
    
    /* Draw a border around the screen */
    VgaDrawRect(0, 0, 640, 480, VGA_COLOR_WHITE);
}
