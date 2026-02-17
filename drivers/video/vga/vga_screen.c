/*
 * PROJECT:     System_Operative_Edit
 * LICENSE:     GPL-3.0
 * PURPOSE:     VGA Screen Operations
 * COPYRIGHT:   Adapted from ReactOS VGA Driver (GPL-3.0)
 *              Original: ReactOS Project (win32ss/drivers/displays/vga/main/screen.c)
 *              Adaptation: Universidad de Guayaquil
 */

#include "vga.h"

/* External functions */
extern VOID VgaWriteGraphicsController(UCHAR Index, UCHAR Value);
extern VOID VgaWriteSequencer(UCHAR Index, UCHAR Value);
extern UCHAR VgaReadGraphicsController(UCHAR Index);

/* Global VGA device */
static PDEVICE_OBJECT g_VgaDevice = NULL;

/**
 * VgaSetDeviceObject - Set global VGA device object
 * @DeviceObject: VGA device object
 */
VOID VgaSetDeviceObject(PDEVICE_OBJECT DeviceObject)
{
    g_VgaDevice = DeviceObject;
}

/**
 * VgaGetDeviceObject - Get VGA device object
 * 
 * Returns: Pointer to VGA device object or NULL
 */
PDEVICE_OBJECT VgaGetDeviceObject(VOID)
{
    return g_VgaDevice;
}

/**
 * VgaPutPixel - Draw a pixel at specified coordinates
 * @x: X coordinate
 * @y: Y coordinate
 * @Color: Pixel color (0-15)
 */
VOID VgaPutPixel(INT x, INT y, UCHAR Color)
{
    PVGA_DEVICE_EXTENSION DevExt;
    PUCHAR FrameBuffer;
    ULONG Offset;
    UCHAR BitMask;
    INT PlaneIndex;
    
    if (!g_VgaDevice) {
        return;
    }
    
    DevExt = (PVGA_DEVICE_EXTENSION)g_VgaDevice->DeviceExtension;
    if (!DevExt || !DevExt->GraphicsMode) {
        return;
    }
    
    /* Bounds checking */
    if (x < 0 || x >= (INT)DevExt->ScreenWidth) {
        return;
    }
    if (y < 0 || y >= (INT)DevExt->ScreenHeight) {
        return;
    }
    
    FrameBuffer = (PUCHAR)DevExt->FrameBuffer;
    
    /* VGA planar mode pixel calculation */
    Offset = y * (DevExt->ScreenWidth / 8) + (x / 8);
    BitMask = 0x80 >> (x % 8);
    
    /* Set bit mask */
    VgaWriteGraphicsController(8, BitMask);
    
    /* Write to all planes */
    for (PlaneIndex = 0; PlaneIndex < 4; PlaneIndex++) {
        /* Select plane */
        VgaWriteSequencer(2, 1 << PlaneIndex);
        
        /* Read to latch */
        volatile UCHAR dummy = FrameBuffer[Offset];
        (void)dummy;
        
        /* Write pixel */
        if (Color & (1 << PlaneIndex)) {
            FrameBuffer[Offset] = 0xFF;
        } else {
            FrameBuffer[Offset] = 0x00;
        }
    }
    
    /* Restore bit mask */
    VgaWriteGraphicsController(8, 0xFF);
    
    /* Restore plane mask */
    VgaWriteSequencer(2, 0x0F);
}

/**
 * VgaClearScreen - Clear screen with specified color
 * @Color: Clear color (0-15)
 */
VOID VgaClearScreen(UCHAR Color)
{
    PVGA_DEVICE_EXTENSION DevExt;
    PUCHAR FrameBuffer;
    ULONG Size;
    INT PlaneIndex;
    
    if (!g_VgaDevice) {
        return;
    }
    
    DevExt = (PVGA_DEVICE_EXTENSION)g_VgaDevice->DeviceExtension;
    if (!DevExt || !DevExt->GraphicsMode) {
        return;
    }
    
    FrameBuffer = (PUCHAR)DevExt->FrameBuffer;
    Size = DevExt->ScreenHeight * (DevExt->ScreenWidth / 8);
    
    /* Clear each plane */
    for (PlaneIndex = 0; PlaneIndex < 4; PlaneIndex++) {
        /* Select plane */
        VgaWriteSequencer(2, 1 << PlaneIndex);
        
        /* Fill plane with color bit */
        UCHAR FillValue = (Color & (1 << PlaneIndex)) ? 0xFF : 0x00;
        for (ULONG i = 0; i < Size; i++) {
            FrameBuffer[i] = FillValue;
        }
    }
    
    /* Restore plane mask */
    VgaWriteSequencer(2, 0x0F);
}

/**
 * VgaFillRect - Fill a rectangle with specified color
 * @x: X coordinate of top-left corner
 * @y: Y coordinate of top-left corner
 * @width: Width of rectangle
 * @height: Height of rectangle
 * @Color: Fill color (0-15)
 */
VOID VgaFillRect(INT x, INT y, INT width, INT height, UCHAR Color)
{
    INT dx, dy;
    
    if (!g_VgaDevice) {
        return;
    }
    
    /* Simple implementation - draw pixel by pixel */
    for (dy = 0; dy < height; dy++) {
        for (dx = 0; dx < width; dx++) {
            VgaPutPixel(x + dx, y + dy, Color);
        }
    }
}

/**
 * VgaDrawLine - Draw a line between two points (Bresenham's algorithm)
 * @x1: Starting X coordinate
 * @y1: Starting Y coordinate
 * @x2: Ending X coordinate
 * @y2: Ending Y coordinate
 * @Color: Line color (0-15)
 */
VOID VgaDrawLine(INT x1, INT y1, INT x2, INT y2, UCHAR Color)
{
    INT dx, dy, sx, sy, err, e2;
    
    if (!g_VgaDevice) {
        return;
    }
    
    dx = (x2 > x1) ? (x2 - x1) : (x1 - x2);
    dy = (y2 > y1) ? (y2 - y1) : (y1 - y2);
    sx = (x1 < x2) ? 1 : -1;
    sy = (y1 < y2) ? 1 : -1;
    err = dx - dy;
    
    while (1) {
        VgaPutPixel(x1, y1, Color);
        
        if (x1 == x2 && y1 == y2) {
            break;
        }
        
        e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}
