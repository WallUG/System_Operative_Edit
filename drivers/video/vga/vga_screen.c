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
    PVGA_DEVICE_EXTENSION DevExt;
    PUCHAR FrameBuffer;
    ULONG Offset;
    UCHAR BitMask;

    if (!g_VgaDevice) return;
    DevExt = (PVGA_DEVICE_EXTENSION)g_VgaDevice->DeviceExtension;
    if (!DevExt || !DevExt->GraphicsMode) return;
    if (x < 0 || x >= (INT)DevExt->ScreenWidth)  return;
    if (y < 0 || y >= (INT)DevExt->ScreenHeight) return;

    FrameBuffer = (PUCHAR)DevExt->FrameBuffer;
    Offset  = (ULONG)y * (DevExt->ScreenWidth / 8) + (ULONG)(x / 8);
    BitMask = 0x80 >> (x & 7);   /* bit7 = pixel izquierdo del byte */

    /*
     * Metodo Set/Reset: la forma canonica de escribir un pixel en VGA planar.
     * El GC aplica el color a todos los planos automaticamente, sin necesidad
     * de iterar plano por plano ni preocuparse por el latch.
     *
     * GC[0] Set/Reset    = Color (el color a escribir)
     * GC[1] Enable S/R   = 0x0F  (activar S/R en los 4 planos)
     * GC[8] Bit Mask     = BitMask (solo el bit del pixel a cambiar)
     * Sequencer[2]       = 0x0F (habilitar escritura en 4 planos)
     *
     * La lectura del latch (dummy read) carga el byte actual del framebuffer
     * en los latches de los 4 planos. Luego la escritura aplica:
     *   - En los bits indicados por BitMask: usa el valor de Set/Reset
     *   - En los demas bits: mantiene el valor del latch (otros pixels)
     * Resultado: solo el pixel en (x,y) cambia, los 7 vecinos no se tocan.
     */
    VgaWriteGraphicsController(0, Color);    /* Set/Reset value */
    VgaWriteGraphicsController(1, 0x0F);     /* Enable Set/Reset todos planos */
    VgaWriteGraphicsController(8, BitMask);  /* Bit mask: solo nuestro pixel */
    VgaWriteSequencer(2, 0x0F);              /* Escribir en los 4 planos */

    /* Latch read: carga los 4 planos en los latches internos */
    volatile UCHAR dummy = FrameBuffer[Offset]; (void)dummy;

    /* Escritura: aplica Set/Reset con BitMask -> escribe el pixel */
    FrameBuffer[Offset] = 0xFF;   /* El valor no importa, S/R lo sobreescribe */

    /* Restaurar GC a estado normal */
    VgaWriteGraphicsController(1, 0x00);     /* Disable Set/Reset */
    VgaWriteGraphicsController(8, 0xFF);     /* Bit mask = todos */

    /* Actualizar shadow framebuffer */
    g_shadow[y * SHADOW_W + x] = Color;
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
    PVGA_DEVICE_EXTENSION DevExt;
    PUCHAR fb;
    INT row, cx;
    INT x_end;
    ULONG stride;

    if (!g_VgaDevice) return;
    DevExt = (PVGA_DEVICE_EXTENSION)g_VgaDevice->DeviceExtension;
    if (!DevExt || !DevExt->GraphicsMode) return;
    if (width <= 0 || height <= 0) return;

    fb     = (PUCHAR)DevExt->FrameBuffer;
    stride = DevExt->ScreenWidth / 8;   /* bytes por fila por plano = 80 */
    x_end  = x + width;

    /* Configurar write mode 0: Set/Reset activo con el color */
    VgaWriteGraphicsController(0, Color);  /* Set/Reset = color */
    VgaWriteGraphicsController(1, 0x0F);   /* Enable Set/Reset en los 4 planos */
    VgaWriteGraphicsController(3, 0x00);   /* Data Rotate = 0 */
    VgaWriteGraphicsController(5, 0x00);   /* Write mode 0 */

    for (row = 0; row < height; row++) {
        INT py = y + row;
        if (py < 0 || py >= (INT)DevExt->ScreenHeight) continue;

        /* Habilitar escritura en todos los planos */
        VgaWriteSequencer(2, 0x0F);

        for (cx = x; cx < x_end; ) {
            if (cx < 0) { cx++; continue; }
            if (cx >= (INT)DevExt->ScreenWidth) break;

            ULONG byte_off = (ULONG)py * stride + (ULONG)cx / 8;
            INT   bit_pos  = cx % 8;

            if (bit_pos == 0 && (cx + 8) <= x_end && (cx + 8) <= (INT)DevExt->ScreenWidth) {
                /* Byte completo: 8 pixeles de una sola escritura */
                VgaWriteGraphicsController(8, 0xFF);   /* todos los bits */
                volatile UCHAR dummy = fb[byte_off]; (void)dummy;
                fb[byte_off] = 0xFF;
                cx += 8;
            } else {
                /* Byte parcial: construir mascara de bits */
                INT  end_bit = 8;
                if ((cx - bit_pos + 8) > x_end)   end_bit = x_end - (cx - bit_pos);
                if ((cx - bit_pos + 8) > (INT)DevExt->ScreenWidth) end_bit = (INT)DevExt->ScreenWidth - (cx - bit_pos);
                UCHAR mask = 0;
                for (INT b = bit_pos; b < end_bit; b++) mask |= (0x80 >> b);
                VgaWriteGraphicsController(8, mask);
                volatile UCHAR dummy = fb[byte_off]; (void)dummy;
                fb[byte_off] = 0xFF;
                cx = (cx - bit_pos) + end_bit;
            }
        }
    }

    /* Restaurar estado normal */
    VgaWriteGraphicsController(0, 0x00);
    VgaWriteGraphicsController(1, 0x00);
    VgaWriteGraphicsController(8, 0xFF);
    VgaWriteSequencer(2, 0x0F);

    /* Actualizar shadow framebuffer para el rectangulo completo */
    {
        INT sy, sx;
        for (sy = y; sy < y + height; sy++) {
            if (sy < 0 || sy >= SHADOW_H) continue;
            for (sx = x; sx < x + width; sx++) {
                if (sx < 0 || sx >= SHADOW_W) continue;
                g_shadow[sy * SHADOW_W + sx] = Color;
            }
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
