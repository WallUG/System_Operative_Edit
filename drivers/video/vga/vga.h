/*
 * PROJECT:     System_Operative_Edit
 * LICENSE:     GPL-3.0
 * PURPOSE:     VGA Driver Definitions and API
 * COPYRIGHT:   Adapted from ReactOS VGA Driver (GPL-3.0)
 *              Original: ReactOS Project (win32ss/drivers/displays/vga)
 *              Adaptation: Universidad de Guayaquil
 */

#ifndef _VGA_H
#define _VGA_H

#include <drivers/ddk/ntddk.h>
#include <drivers/io_manager.h>

/* VGA Hardware Definitions */
#define VGA_BASE_ADDRESS        0xA0000
#define VGA_MEMORY_SIZE         0x10000

/* VGA Register Ports */
#define VGA_CRTC_INDEX          0x3D4
#define VGA_CRTC_DATA           0x3D5
#define VGA_SEQ_INDEX           0x3C4
#define VGA_SEQ_DATA            0x3C5
#define VGA_GC_INDEX            0x3CE
#define VGA_GC_DATA             0x3CF
#define VGA_AC_INDEX            0x3C0
#define VGA_AC_WRITE            0x3C0
#define VGA_AC_READ             0x3C1
#define VGA_MISC_WRITE          0x3C2
#define VGA_MISC_READ           0x3CC
#define VGA_INPUT_STATUS_1      0x3DA

/* VGA Modes */
#define VGA_MODE_TEXT_80x25     0x03
#define VGA_MODE_GRAPHICS_640x480x16    0x12

/* VGA Colors (16-color palette) */
#define VGA_COLOR_BLACK         0
#define VGA_COLOR_BLUE          1
#define VGA_COLOR_GREEN         2
#define VGA_COLOR_CYAN          3
#define VGA_COLOR_RED           4
#define VGA_COLOR_MAGENTA       5
#define VGA_COLOR_BROWN         6
#define VGA_COLOR_LIGHT_GRAY    7
#define VGA_COLOR_DARK_GRAY     8
#define VGA_COLOR_LIGHT_BLUE    9
#define VGA_COLOR_LIGHT_GREEN   10
#define VGA_COLOR_LIGHT_CYAN    11
#define VGA_COLOR_LIGHT_RED     12
#define VGA_COLOR_LIGHT_MAGENTA 13
#define VGA_COLOR_YELLOW        14
#define VGA_COLOR_WHITE         15

/* VGA Device Extension */
typedef struct _VGA_DEVICE_EXTENSION {
    PVOID FrameBuffer;           // Video memory pointer
    ULONG ScreenWidth;           // Current screen width
    ULONG ScreenHeight;          // Current screen height
    ULONG BitsPerPixel;          // Bits per pixel
    BOOLEAN GraphicsMode;        // TRUE if graphics mode
    UCHAR CurrentMode;           // Current video mode
    UCHAR CurrentPalette[16];    // Current palette (16 colors)
} VGA_DEVICE_EXTENSION, *PVGA_DEVICE_EXTENSION;

/* Function declarations */

/**
 * VgaDriverEntry - VGA driver initialization
 * @DriverObject: Driver object
 * @RegistryPath: Registry path
 * 
 * Returns: STATUS_SUCCESS or error code
 */
NTSTATUS VgaDriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
);

/**
 * VgaSetMode - Set VGA display mode
 * @Mode: VGA mode number
 * 
 * Returns: STATUS_SUCCESS or error code
 */
NTSTATUS VgaSetMode(UCHAR Mode);

/**
 * VgaPutPixel - Draw a pixel at specified coordinates
 * @x: X coordinate
 * @y: Y coordinate
 * @Color: Pixel color (0-15)
 */
VOID VgaPutPixel(INT x, INT y, UCHAR Color);

/**
 * VgaFillRect - Fill a rectangle with specified color
 * @x: X coordinate of top-left corner
 * @y: Y coordinate of top-left corner
 * @width: Width of rectangle
 * @height: Height of rectangle
 * @Color: Fill color (0-15)
 */
VOID VgaFillRect(INT x, INT y, INT width, INT height, UCHAR Color);

/**
 * VgaDrawLine - Draw a line between two points
 * @x1: Starting X coordinate
 * @y1: Starting Y coordinate
 * @x2: Ending X coordinate
 * @y2: Ending Y coordinate
 * @Color: Line color (0-15)
 */
VOID VgaDrawLine(INT x1, INT y1, INT x2, INT y2, UCHAR Color);

/**
 * VgaClearScreen - Clear screen with specified color
 * @Color: Clear color (0-15)
 */
VOID VgaClearScreen(UCHAR Color);

/**
 * VgaDrawRect - Draw a rectangle outline
 * @x: X coordinate of top-left corner
 * @y: Y coordinate of top-left corner
 * @width: Width of rectangle
 * @height: Height of rectangle
 * @Color: Border color (0-15)
 */
VOID VgaDrawRect(INT x, INT y, INT width, INT height, UCHAR Color);

/**
 * VgaDrawDemo - Draw a demo pattern to test VGA functionality
 */
VOID VgaDrawDemo(VOID);

/**
 * VgaGetPixel - Leer el color de un pixel desde el shadow framebuffer
 * @x: Coordenada X
 * @y: Coordenada Y
 * Returns: Color del pixel (0-15)
 */
UCHAR VgaGetPixel(INT x, INT y);

/**
 * VgaGetDeviceObject - Get VGA device object
 * 
 * Returns: Pointer to VGA device object or NULL
 */
PDEVICE_OBJECT VgaGetDeviceObject(VOID);

/* Hardware access functions */

/**
 * VgaWriteSequencer - Write to VGA sequencer register
 * @Index: Register index
 * @Value: Value to write
 */
VOID VgaWriteSequencer(UCHAR Index, UCHAR Value);

/**
 * VgaWriteGraphicsController - Write to VGA graphics controller register
 * @Index: Register index
 * @Value: Value to write
 */
VOID VgaWriteGraphicsController(UCHAR Index, UCHAR Value);

/**
 * VgaWriteCrtc - Write to VGA CRTC register
 * @Index: Register index
 * @Value: Value to write
 */
VOID VgaWriteCrtc(UCHAR Index, UCHAR Value);

/**
 * VgaSetPalette - Set VGA palette entry
 * @Index: Palette index (0-15)
 * @Red: Red component (0-63)
 * @Green: Green component (0-63)
 * @Blue: Blue component (0-63)
 */
VOID VgaSetPalette(UCHAR Index, UCHAR Red, UCHAR Green, UCHAR Blue);

#endif /* _VGA_H */
