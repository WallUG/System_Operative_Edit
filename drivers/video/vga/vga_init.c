/*
 * PROJECT:     System_Operative_Edit
 * LICENSE:     GPL-3.0
 * PURPOSE:     VGA Initialization
 * COPYRIGHT:   Adapted from ReactOS VGA Driver (GPL-3.0)
 *              Original: ReactOS Project
 *              Adaptation: Universidad de Guayaquil
 */

#include "vga.h"

/* External hardware functions */
extern VOID VgaInitializePalette(VOID);
extern VOID VgaWriteSequencer(UCHAR Index, UCHAR Value);
extern VOID VgaWriteGraphicsController(UCHAR Index, UCHAR Value);
extern VOID VgaWriteCrtc(UCHAR Index, UCHAR Value);
extern void outb(uint16_t port, uint8_t value);
extern uint8_t inb(uint16_t port);

/**
 * VgaSetMode - Set VGA display mode
 * @Mode: VGA mode number
 * 
 * Returns: STATUS_SUCCESS or error code
 */
NTSTATUS VgaSetMode(UCHAR Mode)
{
    if (Mode == VGA_MODE_GRAPHICS_640x480x16) {
        /* Set mode 0x12: 640x480x16 colors */
        
        /* Use BIOS interrupt to set mode (simplified) */
        /* In a real implementation, we would:
         * 1. Save current mode
         * 2. Program VGA registers directly
         * 3. Set up planar graphics mode
         */
        
        /* For now, we'll program basic registers for mode 0x12 */
        
        /* Miscellaneous Output Register */
        outb(VGA_MISC_WRITE, 0xE3);
        
        /* Sequencer Registers */
        VgaWriteSequencer(0x00, 0x03);  /* Reset */
        VgaWriteSequencer(0x01, 0x01);  /* Clocking Mode */
        VgaWriteSequencer(0x02, 0x0F);  /* Map Mask */
        VgaWriteSequencer(0x03, 0x00);  /* Character Map Select */
        VgaWriteSequencer(0x04, 0x06);  /* Memory Mode */
        
        /* Graphics Controller Registers */
        VgaWriteGraphicsController(0x00, 0x00);  /* Set/Reset */
        VgaWriteGraphicsController(0x01, 0x00);  /* Enable Set/Reset */
        VgaWriteGraphicsController(0x02, 0x00);  /* Color Compare */
        VgaWriteGraphicsController(0x03, 0x00);  /* Data Rotate */
        VgaWriteGraphicsController(0x04, 0x00);  /* Read Map Select */
        VgaWriteGraphicsController(0x05, 0x00);  /* Graphics Mode */
        VgaWriteGraphicsController(0x06, 0x05);  /* Miscellaneous */
        VgaWriteGraphicsController(0x07, 0x0F);  /* Color Don't Care */
        VgaWriteGraphicsController(0x08, 0xFF);  /* Bit Mask */
        
        /* Initialize palette */
        VgaInitializePalette();
        
        return STATUS_SUCCESS;
    }
    else if (Mode == VGA_MODE_TEXT_80x25) {
        /* Text mode - already set by BIOS, just acknowledge */
        return STATUS_SUCCESS;
    }
    
    return STATUS_INVALID_PARAMETER;
}

/**
 * VgaInitializeDevice - Initialize VGA device
 * @DevExt: Device extension
 * 
 * Returns: STATUS_SUCCESS or error code
 */
NTSTATUS VgaInitializeDevice(PVGA_DEVICE_EXTENSION DevExt)
{
    NTSTATUS Status;
    
    if (!DevExt) {
        return STATUS_INVALID_PARAMETER;
    }
    
    /* Set graphics mode */
    Status = VgaSetMode(VGA_MODE_GRAPHICS_640x480x16);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    
    /* Setup device extension */
    DevExt->FrameBuffer = (PVOID)VGA_BASE_ADDRESS;
    DevExt->ScreenWidth = 640;
    DevExt->ScreenHeight = 480;
    DevExt->BitsPerPixel = 4;  /* 4 bits per pixel = 16 colors */
    DevExt->GraphicsMode = TRUE;
    DevExt->CurrentMode = VGA_MODE_GRAPHICS_640x480x16;
    
    /* Initialize palette */
    for (int i = 0; i < 16; i++) {
        DevExt->CurrentPalette[i] = (UCHAR)i;
    }
    
    /* Clear screen to black */
    VgaClearScreen(VGA_COLOR_BLACK);
    
    return STATUS_SUCCESS;
}
