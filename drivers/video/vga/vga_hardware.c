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
 * 
 * Note: Reserved for future use. Will be needed for advanced drawing operations.
 */
VOID VgaSetWriteMode(UCHAR Mode)
{
    UCHAR value = VgaReadGraphicsController(5);
    value = (value & 0xFC) | (Mode & 0x03);
    VgaWriteGraphicsController(5, value);
}

/**
 * VgaSelectPlane - Select VGA plane for writing
 * @Plane: Plane number (0-3)
 * 
 * Note: Reserved for future use. Will be needed for advanced drawing operations.
 */
VOID VgaSelectPlane(UCHAR Plane)
{
    VgaWriteSequencer(2, 1 << Plane);
}

/**
 * VgaSetPlanesMask - Set VGA planes write mask
 * @Mask: Planes mask (bits 0-3)
 * 
 * Note: Reserved for future use. Will be needed for advanced drawing operations.
 */
VOID VgaSetPlanesMask(UCHAR Mask)
{
    VgaWriteSequencer(2, Mask);
}

/**
 * VgaSetBitMask - Set VGA bit mask
 * @Mask: Bit mask
 * 
 * Note: Reserved for future use. Will be needed for advanced drawing operations.
 */
VOID VgaSetBitMask(UCHAR Mask)
{
    VgaWriteGraphicsController(8, Mask);
}

/**
 * VgaHardwareReset - Perform complete VGA hardware reset
 * 
 * This function performs a complete VGA hardware reset to clear any
 * inconsistent state left by previous mode configurations (e.g., bootloader).
 * 
 * The reset sequence:
 * 1. Disable video output (prevents visual glitches during reset)
 * 2. Perform async + sync Sequencer reset
 * 3. Reset Attribute Controller flip-flop
 * 4. Clear Graphics Controller registers to default state
 * 5. Unlock and clear CRTC registers
 * 6. Reset Miscellaneous Output Register
 * 7. Initialize Sequencer for normal operation
 * 
 * This must be called BEFORE any mode-specific register programming.
 */
VOID VgaHardwareReset(VOID)
{
    int i;
    
    /* Step 1: Disable video output via Attribute Controller
     * Reading IS1 resets flip-flop, then write index with bit 5=0 to disable video */
    inb(VGA_INPUT_STATUS_1);
    outb(VGA_AC_INDEX, 0x00);  /* Disable video (bit 5 = 0) */
    
    /* Step 2: Perform complete Sequencer reset
     * Async reset (0x00) stops all sequencer operations
     * Sync reset (0x01) holds sequencer in reset while clearing state
     * Normal operation (0x03) resumes operations with clean state */
    outb(VGA_SEQ_INDEX, 0x00);
    outb(VGA_SEQ_DATA, 0x00);   /* Async reset */
    
    /* Small I/O delay to ensure reset propagates through hardware */
    for (i = 0; i < 5; i++) {
        outb(0x80, 0);
    }
    
    outb(VGA_SEQ_INDEX, 0x00);
    outb(VGA_SEQ_DATA, 0x01);   /* Sync reset */
    
    /* Another delay */
    for (i = 0; i < 5; i++) {
        outb(0x80, 0);
    }
    
    outb(VGA_SEQ_INDEX, 0x00);
    outb(VGA_SEQ_DATA, 0x03);   /* Normal operation */
    
    /* Step 3: Reset Attribute Controller flip-flop state
     * Read IS1 to ensure flip-flop is in known state */
    inb(VGA_INPUT_STATUS_1);
    
    /* Step 4: Reset Graphics Controller to safe defaults
     * Clear shift/rotate modes, set standard write mode */
    VgaWriteGraphicsController(0, 0x00);  /* Set/Reset: 0 */
    VgaWriteGraphicsController(1, 0x00);  /* Enable Set/Reset: 0 */
    VgaWriteGraphicsController(2, 0x00);  /* Color Compare: 0 */
    VgaWriteGraphicsController(3, 0x00);  /* Data Rotate: no rotation, replace */
    VgaWriteGraphicsController(4, 0x00);  /* Read Map Select: plane 0 */
    VgaWriteGraphicsController(5, 0x00);  /* Graphics Mode: write mode 0, read mode 0 */
    VgaWriteGraphicsController(6, 0x00);  /* Miscellaneous: text mode addressing */
    VgaWriteGraphicsController(7, 0x0F);  /* Color Don't Care: all bits */
    VgaWriteGraphicsController(8, 0xFF);  /* Bit Mask: all bits */
    
    /* Step 5: Unlock CRTC registers and clear critical ones
     * CRTC register 0x11 bit 7 locks registers 0-7 */
    outb(VGA_CRTC_INDEX, 0x11);
    outb(VGA_CRTC_DATA, 0x00);  /* Unlock CRTC, clear Vertical Retrace End */
    
    /* Clear start address registers (prevents display offset issues) */
    VgaWriteCrtc(0x0C, 0x00);   /* Start Address High */
    VgaWriteCrtc(0x0D, 0x00);   /* Start Address Low */
    
    /* Step 6: Reset Miscellaneous Output Register to default
     * This controls I/O address selection and sync polarities
     * 0x63 = bit0: I/O @ 3Dx (not 3Bx), bit1: RAM enabled, 
     *        bits2-3: 25MHz dot clock, bit5: Vsync-, bit6: Hsync- */
    outb(VGA_MISC_WRITE, 0x63);
    
    /* Step 7: Initialize Sequencer for standard operation */
    VgaWriteSequencer(1, 0x00);  /* Clocking Mode: standard */
    VgaWriteSequencer(2, 0x0F);  /* Map Mask: all planes enabled */
    VgaWriteSequencer(3, 0x00);  /* Character Map Select: default */
    /* Memory Mode: 0x02 = bit1 set (extended memory enabled), 
     * bit2 clear (odd/even disabled), bit3 clear (Chain 4 disabled) */
    VgaWriteSequencer(4, 0x02);
    
    /* Final flip-flop reset before returning */
    inb(VGA_INPUT_STATUS_1);
}
