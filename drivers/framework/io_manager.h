/*
 * PROJECT:     System_Operative_Edit
 * LICENSE:     GPL-3.0
 * PURPOSE:     I/O Manager Framework Header
 * COPYRIGHT:   Adapted from ReactOS (GPL-3.0)
 *              Original: ReactOS Project
 *              Adaptation: Universidad de Guayaquil
 */

#ifndef _IO_MANAGER_INTERNAL_H
#define _IO_MANAGER_INTERNAL_H

#include <drivers/io_manager.h>

/* Global driver list management */
extern PDRIVER_OBJECT g_DriverList;
extern int g_DriverCount;

/* Internal functions */
NTSTATUS IopCreateDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN ULONG DeviceExtensionSize,
    IN PUNICODE_STRING DeviceName OPTIONAL,
    IN DEVICE_TYPE DeviceType,
    IN ULONG DeviceCharacteristics,
    IN BOOLEAN Exclusive,
    OUT PDEVICE_OBJECT *DeviceObject
);

VOID IopDeleteDevice(
    IN PDEVICE_OBJECT DeviceObject
);

#endif /* _IO_MANAGER_INTERNAL_H */
