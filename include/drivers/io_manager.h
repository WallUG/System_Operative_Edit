/*
 * PROJECT:     System_Operative_Edit
 * LICENSE:     GPL-3.0
 * PURPOSE:     I/O Manager Public API
 * COPYRIGHT:   Adapted from ReactOS (GPL-3.0)
 *              Original: ReactOS Project
 *              Adaptation: Universidad de Guayaquil
 */

#ifndef _IO_MANAGER_H
#define _IO_MANAGER_H

#include <drivers/ddk/wdm.h>

/**
 * IoCreateDriver - Register a new driver
 * @DriverName: Name of the driver
 * @DriverInit: Initialization function
 * 
 * Returns: STATUS_SUCCESS or error code
 */
NTSTATUS IoCreateDriver(
    IN PUNICODE_STRING DriverName,
    IN PDRIVER_INITIALIZE DriverInit
);

/**
 * IoDeleteDriver - Unregister a driver
 * @DriverObject: Driver object to delete
 */
VOID IoDeleteDriver(
    IN PDRIVER_OBJECT DriverObject
);

/**
 * IoCreateDevice - Create a device object
 * @DriverObject: Driver object that owns this device
 * @DeviceExtensionSize: Size of device extension
 * @DeviceName: Optional device name
 * @DeviceType: Type of device
 * @DeviceCharacteristics: Device characteristics
 * @Exclusive: TRUE if exclusive access
 * @DeviceObject: Receives created device object
 * 
 * Returns: STATUS_SUCCESS or error code
 */
NTSTATUS IoCreateDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN ULONG DeviceExtensionSize,
    IN PUNICODE_STRING DeviceName OPTIONAL,
    IN DEVICE_TYPE DeviceType,
    IN ULONG DeviceCharacteristics,
    IN BOOLEAN Exclusive,
    OUT PDEVICE_OBJECT *DeviceObject
);

/**
 * IoDeleteDevice - Delete a device object
 * @DeviceObject: Device object to delete
 */
VOID IoDeleteDevice(
    IN PDEVICE_OBJECT DeviceObject
);

/**
 * IoGetDeviceObjectPointer - Get pointer to device object
 * @DeviceName: Name of device
 * @DesiredAccess: Access rights
 * @FileObject: Receives file object
 * @DeviceObject: Receives device object
 * 
 * Returns: STATUS_SUCCESS or error code
 */
NTSTATUS IoGetDeviceObjectPointer(
    IN PUNICODE_STRING DeviceName,
    IN ULONG DesiredAccess,
    OUT PVOID *FileObject,
    OUT PDEVICE_OBJECT *DeviceObject
);

/**
 * IoInitSystem - Initialize I/O Manager
 * 
 * Returns: STATUS_SUCCESS or error code
 */
NTSTATUS IoInitSystem(VOID);

#endif /* _IO_MANAGER_H */
