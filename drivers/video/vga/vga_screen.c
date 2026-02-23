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

/*
 * Shadow framebuffer — copia en RAM del estado logico de cada pixel.
 * 640 x 480 = 307200 bytes (~300 KB).
 * Permite leer el color de cualquier pixel sin acceder al hardware VGA
 * (que no soporta lectura directa de color en modo planar de forma simple).
 * VgaPutPixel y VgaFillRect actualizan este buffer junto con el hardware.
 */
#define SHADOW_W 640
#define SHADOW_H 480
static UCHAR g_shadow[SHADOW_W * SHADOW_H];

UCHAR VgaGetPixel(INT x, INT y)
{
    if (x < 0 || x >= SHADOW_W || y < 0 || y >= SHADOW_H) return 0;
    return g_shadow[y * SHADOW_W + x];
}

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
    /* esta función se llama desde múltiples sitios, evitar el depurador una vez
       el sistema haya arrancado para no desperdiciar tiempo en I/O serial. */
    (void)x; (void)y; (void)Color;

    PVGA_DEVICE_EXTENSION DevExt;
    PUCHAR FrameBuffer;

    if (!g_VgaDevice) return;
    DevExt = (PVGA_DEVICE_EXTENSION)g_VgaDevice->DeviceExtension;
    if (!DevExt || !DevExt->GraphicsMode) return;
    if (x < 0 || x >= (INT)DevExt->ScreenWidth)  return;
    if (y < 0 || y >= (INT)DevExt->ScreenHeight) return;

    FrameBuffer = (PUCHAR)DevExt->FrameBuffer;

    /* compute byte index for (x,y) */
    ULONG byte = (ULONG)y * (DevExt->ScreenWidth / 8) + (ULONG)(x / 8);
    UCHAR bitmask = 0x80 >> (x & 7);

    /* update shadow with new color */
    g_shadow[y * SHADOW_W + x] = Color;

    /* for each plane, rebuild the byte from shadow and write with mask */
    for (int p = 0; p < 4; p++) {
        UCHAR planeValue = 0;
        int baseX = (x / 8) * 8;
        for (int b = 0; b < 8; b++) {
            int px = baseX + b;
            if (px < 0 || px >= SHADOW_W) continue;
            UCHAR col = g_shadow[y * SHADOW_W + px];
            if (col & (1 << p))
                planeValue |= (0x80 >> b);
        }
        /* previously we logged plane values for debugging; removed in production */

        /* select plane via sequencer mask */
        VgaWriteSequencer(2, 1 << p);
        FrameBuffer[byte] = planeValue;
    }

    /* restore mask so future writes hit all planes */
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

    /* Limpiar shadow framebuffer */
    {
        ULONG total = SHADOW_W * SHADOW_H;
        for (ULONG i = 0; i < total; i++) g_shadow[i] = Color;
    }
}

/**
 * VgaFillRect - Fill a rectangle with specified color
 * Usa write mode 0 con Map Mask para llenar 8 pixeles por escritura
 * (hasta 8x mas rapido que pixel-por-pixel)
 */
VOID VgaFillRect(INT x, INT y, INT width, INT height, UCHAR Color)
{
    if (width <= 0 || height <= 0) return;

    /* actualizar shadow primero */
    for (INT row = 0; row < height; row++) {
        int yy = y + row;
        if (yy < 0 || yy >= SHADOW_H) continue;
        for (INT col = 0; col < width; col++) {
            int xx = x + col;
            if (xx < 0 || xx >= SHADOW_W) continue;
            g_shadow[yy * SHADOW_W + xx] = Color;
        }
    }

    /* intentar rellenar por bytes cuando esté alineado a 8 píxeles */
    if (!g_VgaDevice) return;
    PVGA_DEVICE_EXTENSION DevExt = (PVGA_DEVICE_EXTENSION)g_VgaDevice->DeviceExtension;
    if (!DevExt || !DevExt->GraphicsMode) return;

    PUCHAR FrameBuffer = (PUCHAR)DevExt->FrameBuffer;
    int screenW = DevExt->ScreenWidth;
    int bytes_per_row = screenW / 8;

    for (INT row = 0; row < height; row++) {
        int yy = y + row;
        if (yy < 0 || yy >= DevExt->ScreenHeight) continue;
        PUCHAR dest = FrameBuffer + (ULONG)yy * bytes_per_row;
        int start = x;
        int end = x + width;

        /* manejar parte inicial no alineada a 8 */
        while (start < end && (start & 7)) {
            VgaPutPixel(start, yy, Color);
            start++;
        }
        /* manejar segmentos completos de 8 píxeles */
        if (start + 8 <= end) {
            int fullBytes = (end - start) / 8;
            UCHAR planeMask[4];
            for (int p = 0; p < 4; p++)
                planeMask[p] = (Color & (1 << p)) ? 0xFF : 0x00;
            int baseIndex = start / 8;
            for (int p = 0; p < 4; p++) {
                VgaWriteSequencer(2, 1 << p);
                PUCHAR ptr = dest + baseIndex;
                for (int b = 0; b < fullBytes; b++) {
                    ptr[b] = planeMask[p];
                }
            }
            VgaWriteSequencer(2, 0x0F);
            start += fullBytes * 8;
        }
        /* manejar parte final parcial */
        while (start < end) {
            VgaPutPixel(start, yy, Color);
            start++;
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
