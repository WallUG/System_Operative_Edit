/*
 * PROJECT:     System_Operative_Edit
 * LICENSE:     GPL-3.0
 * PURPOSE:     NT Driver Development Kit Essentials
 * COPYRIGHT:   Adapted from ReactOS (GPL-3.0)
 *              Original: ReactOS Project
 *              Adaptation: Universidad de Guayaquil
 */

#ifndef _NTDDK_H
#define _NTDDK_H

#include "wdm.h"

/* Video port definitions */
typedef struct _VIDEO_MODE_INFORMATION {
    ULONG Length;
    ULONG ModeIndex;
    ULONG VisScreenWidth;
    ULONG VisScreenHeight;
    ULONG ScreenStride;
    ULONG NumberOfPlanes;
    ULONG BitsPerPlane;
    ULONG Frequency;
    ULONG XMillimeter;
    ULONG YMillimeter;
    ULONG NumberRedBits;
    ULONG NumberGreenBits;
    ULONG NumberBlueBits;
    ULONG RedMask;
    ULONG GreenMask;
    ULONG BlueMask;
    ULONG AttributeFlags;
    ULONG VideoMemoryBitmapWidth;
    ULONG VideoMemoryBitmapHeight;
} VIDEO_MODE_INFORMATION, *PVIDEO_MODE_INFORMATION;

/* Video mode attributes */
#define VIDEO_MODE_COLOR               0x0001
#define VIDEO_MODE_GRAPHICS            0x0002
#define VIDEO_MODE_PALETTE_DRIVEN      0x0004
#define VIDEO_MODE_MANAGED_PALETTE     0x0008

/* Additional NT types */
typedef struct _LARGE_INTEGER {
    LONG LowPart;
    LONG HighPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

/* Synchronization */
#define KeInitializeSpinLock(Lock) (*(Lock) = 0)
#define KeAcquireSpinLock(Lock, OldIrql) do { } while(0)
#define KeReleaseSpinLock(Lock, OldIrql) do { } while(0)

typedef ULONG KSPIN_LOCK;
typedef ULONG KIRQL;

#endif /* _NTDDK_H */
