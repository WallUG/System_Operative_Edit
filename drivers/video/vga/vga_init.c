/*
 * PROJECT:     System_Operative_Edit
 * LICENSE:     GPL-3.0
 * PURPOSE:     VGA Initialization - Modo 640x480x16 (modo 0x12)
 */

#include "vga.h"

extern VOID VgaInitializePalette(VOID);
extern VOID VgaWriteSequencer(UCHAR Index, UCHAR Value);
extern VOID VgaWriteGraphicsController(UCHAR Index, UCHAR Value);
extern VOID VgaWriteCrtc(UCHAR Index, UCHAR Value);
extern VOID VgaClearScreen(UCHAR Color);
extern VOID VgaHardwareReset(VOID);

static inline void outb_vga(uint16_t port, uint8_t value)
{
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb_vga(uint16_t port)
{
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

/*
 * Tabla completa de registros para modo 640x480x16 (VGA modo 0x12)
 * Valores estandar de la especificacion VGA original de IBM
 */

/* Sequencer registers [index] = value */
static const UCHAR seq_regs[] = {
    0x03,   /* [0] Reset: normal operation */
    0x01,   /* [1] Clocking Mode: 8 dot/char, bit3=0(NO Shift4), bit4=0(NO Shift Load) */
    0x0F,   /* [2] Map Mask: all planes enabled */
    0x00,   /* [3] Character Map Select */
    0x06,   /* [4] Memory Mode: chain4 off, odd/even off, extended mem */
};

/* CRTC registers [index] = value */
static const UCHAR crtc_regs[] = {
    0x5F,   /* [0]  Horizontal Total */
    0x4F,   /* [1]  Horizontal Display End */
    0x50,   /* [2]  Start Horizontal Blanking */
    0x82,   /* [3]  End Horizontal Blanking */
    0x54,   /* [4]  Start Horizontal Retrace */
    0x80,   /* [5]  End Horizontal Retrace */
    0x0B,   /* [6]  Vertical Total: 0x0B correcto para 640x480x16 (modo 0x12) */
    0x3E,   /* [7]  Overflow */
    0x00,   /* [8]  Preset Row Scan */
    0x40,   /* [9]  Max Scan Line */
    0x00,   /* [10] Cursor Start */
    0x00,   /* [11] Cursor End */
    0x00,   /* [12] Start Address High */
    0x00,   /* [13] Start Address Low */
    0x00,   /* [14] Cursor Location High */
    0x00,   /* [15] Cursor Location Low */
    0xEA,   /* [16] Vertical Retrace Start */
    0x8C,   /* [17] Vertical Retrace End */
    0xDF,   /* [18] Vertical Display End */
    0x28,   /* [19] Offset */
    0x00,   /* [20] Underline Location */
    0xE7,   /* [21] Start Vertical Blanking */
    0x04,   /* [22] End Vertical Blanking */
    0xE3,   /* [23] CRT Mode Control */
    0xFF,   /* [24] Line Compare */
};

/* Graphics Controller registers [index] = value */
static const UCHAR gc_regs[] = {
    0x00,   /* [0] Set/Reset */
    0x00,   /* [1] Enable Set/Reset */
    0x00,   /* [2] Color Compare */
    0x00,   /* [3] Data Rotate: sin rotacion, operacion replace */
    0x00,   /* [4] Read Map Select: plano 0 */
    0x00,   /* [5] Graphics Mode: write mode 0, read mode 0, bits[4:3]=00 NO shift */
    0x05,   /* [6] Miscellaneous: bit0=1 graphics, bit1=0 no chain odd/even, A000-AFFF */
    0x0F,   /* [7] Color Don't Care */
    0xFF,   /* [8] Bit Mask: todos los bits */
};

/* Attribute Controller registers [index] = value
 * VALORES CORRECTOS para modo 0x12 (640x480x16) segun especificacion IBM VGA.
 * Los valores 0x38-0x3F en paletas 8-15 eran incorrectos y causaban
 * el efecto espejo horizontal al mapear colores erroneamente. */
static const UCHAR ac_regs[] = {
    /* [0-15] Paleta: mapeo 1:1 de indice a color DAC (0-15) */
    0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F,
    0x01,   /* [16] Mode Control: 0x01 = graphics mode, alpha disable, NO 8-bit (bit6=0 fixes mirror) */
    0x00,   /* [17] Overscan Color: negro */
    0x0F,   /* [18] Color Plane Enable: habilitar los 4 planos */
    0x00,   /* [19] Horizontal Pixel Panning: sin desplazamiento */
    0x00,   /* [20] Color Select */
};

NTSTATUS VgaSetMode(UCHAR Mode)
{
    int i;

    if (Mode == VGA_MODE_GRAPHICS_640x480x16)
    {
        /* 0. COMPLETE HARDWARE RESET
         *    Perform full VGA hardware reset to clear any inconsistent state
         *    left by bootloader (Mode 13h or text mode). This ensures all
         *    VGA registers (Sequencer, CRTC, Graphics Controller, Attribute
         *    Controller) are in a known, clean state before mode programming. */
        VgaHardwareReset();

        /* 1. Desbloquear CRTC registers 0-7 */
        outb_vga(VGA_CRTC_INDEX, 0x11);
        outb_vga(VGA_CRTC_DATA,  inb_vga(VGA_CRTC_DATA) & ~0x80);

        /* 2. Miscellaneous Output Register */
        outb_vga(VGA_MISC_WRITE, 0xE3);

        /* 3. Sequencer */
        for (i = 0; i < 5; i++) {
            VgaWriteSequencer((UCHAR)i, seq_regs[i]);
        }

        /* 4. CRTC */
        for (i = 0; i < 25; i++) {
            VgaWriteCrtc((UCHAR)i, crtc_regs[i]);
        }

        /* 5. Graphics Controller */
        for (i = 0; i < 9; i++) {
            VgaWriteGraphicsController((UCHAR)i, gc_regs[i]);
        }

        /* 6. Attribute Controller
         * PROTOCOLO CORRECTO: leer IS1 UNA SOLA VEZ antes del loop para
         * resetear el flip-flop. Dentro del loop SOLO escribir INDEX+DATA
         * sin leer IS1 de nuevo — leerlo en cada iteracion desfasa el
         * flip-flop y programa el valor en el registro equivocado,
         * causando el efecto espejo horizontal. */
        inb_vga(VGA_INPUT_STATUS_1);            /* reset flip-flop UNA VEZ */
        for (i = 0; i < 21; i++) {
            outb_vga(VGA_AC_INDEX, (UCHAR)i);   /* indice (flip-flop -> INDEX) */
            outb_vga(VGA_AC_WRITE, ac_regs[i]); /* valor  (flip-flop -> DATA)  */
        }

        /* 7. Habilitar video en el AC (bit 5 = 1, sin seleccionar registro) */
        inb_vga(VGA_INPUT_STATUS_1);
        outb_vga(VGA_AC_INDEX, 0x20);  /* bit5=1 = video habilitado */

        /* 8. Paleta de colores estandar */
        VgaInitializePalette();

        return STATUS_SUCCESS;
    }
    else if (Mode == VGA_MODE_TEXT_80x25)
    {
        /* Modo texto ya configurado por GRUB, no hacer nada */
        return STATUS_SUCCESS;
    }

    return STATUS_INVALID_PARAMETER;
}

NTSTATUS VgaInitializeDevice(PVGA_DEVICE_EXTENSION DevExt)
{
    NTSTATUS Status;

    if (!DevExt) {
        return STATUS_INVALID_PARAMETER;
    }

    Status = VgaSetMode(VGA_MODE_GRAPHICS_640x480x16);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    DevExt->FrameBuffer   = (PVOID)VGA_BASE_ADDRESS;
    DevExt->ScreenWidth   = 640;
    DevExt->ScreenHeight  = 480;
    DevExt->BitsPerPixel  = 4;
    DevExt->GraphicsMode  = TRUE;
    DevExt->CurrentMode   = VGA_MODE_GRAPHICS_640x480x16;

    for (int i = 0; i < 16; i++) {
        DevExt->CurrentPalette[i] = (UCHAR)i;
    }

    /* Limpiar pantalla a negro */
    VgaClearScreen(VGA_COLOR_BLACK);

    return STATUS_SUCCESS;
}
