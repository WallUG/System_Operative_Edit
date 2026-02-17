/*
 * PROJECT:     System_Operative_Edit
 * LICENSE:     GPL-3.0
 * PURPOSE:     Windows Driver Model Definitions
 * COPYRIGHT:   Adapted from ReactOS (GPL-3.0)
 *              Original: ReactOS Project
 *              Adaptation: Universidad de Guayaquil
 */

#ifndef _WDM_H
#define _WDM_H

/* Use our own types instead of stdint */
#include <types.h>

/* Basic types */
typedef int32_t NTSTATUS;
typedef void *PVOID;
typedef const void *PCVOID;
typedef uint32_t ULONG;
typedef int32_t LONG;
typedef uint8_t UCHAR;
typedef uint8_t BOOLEAN;
typedef int32_t INT;
typedef uint16_t USHORT;
typedef char CHAR;
typedef unsigned short WCHAR;  /* Define WCHAR as unsigned short */
typedef void VOID;

/* Pointer types */
typedef UCHAR *PUCHAR;
typedef CHAR *PCHAR;
typedef WCHAR *PWCHAR;
typedef ULONG *PULONG;

/* Input/Output parameter modifiers */
#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef OPTIONAL
#define OPTIONAL
#endif

/* Status codes */
#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)
#define STATUS_INSUFFICIENT_RESOURCES    ((NTSTATUS)0xC000009AL)
#define STATUS_INVALID_PARAMETER         ((NTSTATUS)0xC000000DL)
#define STATUS_NOT_IMPLEMENTED           ((NTSTATUS)0xC0000002L)
#define STATUS_DEVICE_DOES_NOT_EXIST     ((NTSTATUS)0xC00000C0L)
#define STATUS_INVALID_DEVICE_REQUEST    ((NTSTATUS)0xC0000010L)
#define STATUS_NO_MEMORY                 ((NTSTATUS)0xC0000017L)

#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)

/* Unicode string */
typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWCHAR Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef const UNICODE_STRING *PCUNICODE_STRING;

/* Device types */
#define FILE_DEVICE_VIDEO           0x00000032
#define FILE_DEVICE_KEYBOARD        0x0000000B
#define FILE_DEVICE_MOUSE           0x0000000F
#define FILE_DEVICE_UNKNOWN         0x00000022

/* Device characteristics */
#define FILE_DEVICE_SECURE_OPEN     0x00000100

/* Forward declarations */
typedef struct _DRIVER_OBJECT DRIVER_OBJECT, *PDRIVER_OBJECT;
typedef struct _DEVICE_OBJECT DEVICE_OBJECT, *PDEVICE_OBJECT;
typedef struct _IRP IRP, *PIRP;
typedef struct _IO_STACK_LOCATION IO_STACK_LOCATION, *PIO_STACK_LOCATION;

/* IRP Major Function Codes */
#define IRP_MJ_CREATE                   0x00
#define IRP_MJ_CLOSE                    0x01
#define IRP_MJ_READ                     0x02
#define IRP_MJ_WRITE                    0x03
#define IRP_MJ_DEVICE_CONTROL           0x0E
#define IRP_MJ_POWER                    0x16
#define IRP_MJ_SYSTEM_CONTROL           0x0E
#define IRP_MJ_PNP                      0x1B
#define IRP_MJ_MAXIMUM_FUNCTION         0x1B

/* Driver initialization function */
typedef NTSTATUS (*PDRIVER_INITIALIZE)(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
);

/* Driver dispatch function */
typedef NTSTATUS (*PDRIVER_DISPATCH)(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
);

/* Driver unload function */
typedef VOID (*PDRIVER_UNLOAD)(
    IN PDRIVER_OBJECT DriverObject
);

/* Driver object structure */
struct _DRIVER_OBJECT {
    UNICODE_STRING DriverName;
    PDEVICE_OBJECT DeviceObject;
    PDRIVER_DISPATCH MajorFunction[IRP_MJ_MAXIMUM_FUNCTION + 1];
    PDRIVER_INITIALIZE DriverInit;
    PDRIVER_UNLOAD DriverUnload;
    PDRIVER_OBJECT NextDriver;
    PVOID DriverExtension;
};

/* Device object structure */
typedef ULONG DEVICE_TYPE;

struct _DEVICE_OBJECT {
    DEVICE_TYPE DeviceType;
    ULONG Characteristics;
    PVOID DeviceExtension;
    UNICODE_STRING DeviceName;
    PDRIVER_OBJECT DriverObject;
    PDEVICE_OBJECT NextDevice;
    PDEVICE_OBJECT AttachedDevice;
    ULONG Flags;
};

/* I/O Request Packet structure (simplified) */
struct _IRP {
    PVOID UserBuffer;
    ULONG IoStatus;
    PVOID AssociatedIrp;
    PIO_STACK_LOCATION CurrentStackLocation;
    PDEVICE_OBJECT DeviceObject;
};

/* I/O Stack Location */
struct _IO_STACK_LOCATION {
    UCHAR MajorFunction;
    UCHAR MinorFunction;
    PDEVICE_OBJECT DeviceObject;
    PVOID Parameters;
};

/* Helper macros */
#define RtlInitUnicodeString(dest, src) \
    do { \
        if ((src) == NULL) { \
            (dest)->Buffer = NULL; \
            (dest)->Length = 0; \
            (dest)->MaximumLength = 0; \
        } else { \
            size_t len = 0; \
            const WCHAR *p = (src); \
            while (*p++) len++; \
            (dest)->Buffer = (PWCHAR)(src); \
            (dest)->Length = (USHORT)(len * sizeof(WCHAR)); \
            (dest)->MaximumLength = (USHORT)((len + 1) * sizeof(WCHAR)); \
        } \
    } while(0)

#define IoGetCurrentIrpStackLocation(Irp) \
    ((Irp)->CurrentStackLocation)

/* Memory allocation macros */
#define ExAllocatePool(PoolType, Size) malloc(Size)
#define ExFreePool(Buffer) free(Buffer)

/* Pool types */
#define NonPagedPool 0
#define PagedPool    1

#endif /* _WDM_H */
