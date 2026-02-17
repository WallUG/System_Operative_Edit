/*
 * PROJECT:     System_Operative_Edit
 * LICENSE:     GPL-3.0
 * PURPOSE:     VGA Hardware Access Functions
 * COPYRIGHT:   Adapted from ReactOS VGA Driver (GPL-3.0)
 *              Original: ReactOS Project (win32ss/drivers/miniport/vga)
 *              Adaptation: Universidad de Guayaquil
 */

#include "vga.h"

/* Port I/O functions - these need to be implemented in assembly or use inline asm */
static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

/**
 * VgaWriteSequencer - Write to VGA sequencer register
 * @Index: Register index
 * @Value: Value to write
 */
VOID VgaWriteSequencer(UCHAR Index, UCHAR Value)
{
    outb(VGA_SEQ_INDEX, Index);
    outb(VGA_SEQ_DATA, Value);
}

/**
 * VgaWriteGraphicsController - Write to VGA graphics controller register
 * @Index: Register index
 * @Value: Value to write
 */
VOID VgaWriteGraphicsController(UCHAR Index, UCHAR Value)
{
    outb(VGA_GC_INDEX, Index);
    outb(VGA_GC_DATA, Value);
}

/**
 * VgaWriteCrtc - Write to VGA CRTC register
 * @Index: Register index
 * @Value: Value to write
 */
VOID VgaWriteCrtc(UCHAR Index, UCHAR Value)
{
    outb(VGA_CRTC_INDEX, Index);
    outb(VGA_CRTC_DATA, Value);
}

/**
 * VgaReadGraphicsController - Read from VGA graphics controller register
 * @Index: Register index
 * 
 * Returns: Register value
 */
UCHAR VgaReadGraphicsController(UCHAR Index)
{
    outb(VGA_GC_INDEX, Index);
    return inb(VGA_GC_DATA);
}

/**
 * VgaSetPalette - Set VGA palette entry
 * @Index: Palette index (0-15)
 * @Red: Red component (0-63)
 * @Green: Green component (0-63)
 * @Blue: Blue component (0-63)
 */
VOID VgaSetPalette(UCHAR Index, UCHAR Red, UCHAR Green, UCHAR Blue)
{
    /* VGA palette programming */
    outb(0x3C8, Index);
    outb(0x3C9, Red);
    outb(0x3C9, Green);
    outb(0x3C9, Blue);
}

/**
 * VgaInitializePalette - Initialize default VGA palette
 */
VOID VgaInitializePalette(VOID)
{
    /* Standard VGA 16-color palette */
    VgaSetPalette(0, 0, 0, 0);           // Black
    VgaSetPalette(1, 0, 0, 42);          // Blue
    VgaSetPalette(2, 0, 42, 0);          // Green
    VgaSetPalette(3, 0, 42, 42);         // Cyan
    VgaSetPalette(4, 42, 0, 0);          // Red
    VgaSetPalette(5, 42, 0, 42);         // Magenta
    VgaSetPalette(6, 42, 21, 0);         // Brown
    VgaSetPalette(7, 42, 42, 42);        // Light Gray
    VgaSetPalette(8, 21, 21, 21);        // Dark Gray
    VgaSetPalette(9, 21, 21, 63);        // Light Blue
    VgaSetPalette(10, 21, 63, 21);       // Light Green
    VgaSetPalette(11, 21, 63, 63);       // Light Cyan
    VgaSetPalette(12, 63, 21, 21);       // Light Red
    VgaSetPalette(13, 63, 21, 63);       // Light Magenta
    VgaSetPalette(14, 63, 63, 21);       // Yellow
    VgaSetPalette(15, 63, 63, 63);       // White
}

/**
 * VgaSetWriteMode - Set VGA write mode
 * @Mode: Write mode (0-3)
 */
static VOID VgaSetWriteMode(UCHAR Mode)
{
    UCHAR value = VgaReadGraphicsController(5);
    value = (value & 0xFC) | (Mode & 0x03);
    VgaWriteGraphicsController(5, value);
}

/**
 * VgaSelectPlane - Select VGA plane for writing
 * @Plane: Plane number (0-3)
 */
static VOID VgaSelectPlane(UCHAR Plane)
{
    VgaWriteSequencer(2, 1 << Plane);
}

/**
 * VgaSetPlanesMask - Set VGA planes write mask
 * @Mask: Planes mask (bits 0-3)
 */
static VOID VgaSetPlanesMask(UCHAR Mask)
{
    VgaWriteSequencer(2, Mask);
}

/**
 * VgaSetBitMask - Set VGA bit mask
 * @Mask: Bit mask
 */
static VOID VgaSetBitMask(UCHAR Mask)
{
    VgaWriteGraphicsController(8, Mask);
}
