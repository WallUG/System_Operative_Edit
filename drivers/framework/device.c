/*
 * PROJECT:     System_Operative_Edit
 * LICENSE:     GPL-3.0
 * PURPOSE:     I/O Manager - Device Management
 * COPYRIGHT:   Adapted from ReactOS (GPL-3.0)
 *              Original: ReactOS Project (ntoskrnl/io/iomgr/device.c)
 *              Adaptation: Universidad de Guayaquil
 */

#include "io_manager.h"
#include <kstdlib.h>

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
)
{
    PDEVICE_OBJECT Device;
    PVOID Extension;
    
    if (!DriverObject || !DeviceObject) {
        return STATUS_INVALID_PARAMETER;
    }
    
    /* Allocate device object with extension */
    Device = (PDEVICE_OBJECT)malloc(sizeof(DEVICE_OBJECT));
    if (!Device) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
    memset(Device, 0, sizeof(DEVICE_OBJECT));
    
    /* Allocate device extension if requested */
    if (DeviceExtensionSize > 0) {
        Extension = malloc(DeviceExtensionSize);
        if (!Extension) {
            free(Device);
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        memset(Extension, 0, DeviceExtensionSize);
        Device->DeviceExtension = Extension;
    } else {
        Device->DeviceExtension = NULL;
    }
    
    /* Initialize device object */
    Device->DriverObject = DriverObject;
    Device->DeviceType = DeviceType;
    Device->Characteristics = DeviceCharacteristics;
    Device->Flags = 0;
    Device->AttachedDevice = NULL;
    Device->NextDevice = NULL;
    
    /* Copy device name if provided */
    if (DeviceName && DeviceName->Buffer) {
        Device->DeviceName = *DeviceName;
    } else {
        Device->DeviceName.Buffer = NULL;
        Device->DeviceName.Length = 0;
        Device->DeviceName.MaximumLength = 0;
    }
    
    /* Link to driver's device list */
    Device->NextDevice = DriverObject->DeviceObject;
    DriverObject->DeviceObject = Device;
    
    *DeviceObject = Device;
    return STATUS_SUCCESS;
}

/**
 * IoDeleteDevice - Delete a device object
 * @DeviceObject: Device object to delete
 */
VOID IoDeleteDevice(
    IN PDEVICE_OBJECT DeviceObject
)
{
    PDRIVER_OBJECT Driver;
    PDEVICE_OBJECT *Current;
    
    if (!DeviceObject) {
        return;
    }
    
    /* Unlink from driver's device list */
    Driver = DeviceObject->DriverObject;
    if (Driver) {
        Current = &Driver->DeviceObject;
        while (*Current) {
            if (*Current == DeviceObject) {
                *Current = DeviceObject->NextDevice;
                break;
            }
            Current = &(*Current)->NextDevice;
        }
    }
    
    /* Free device extension if allocated */
    if (DeviceObject->DeviceExtension) {
        free(DeviceObject->DeviceExtension);
    }
    
    /* Free device object */
    free(DeviceObject);
}

/**
 * IopCreateDevice - Internal device creation (alias)
 */
NTSTATUS IopCreateDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN ULONG DeviceExtensionSize,
    IN PUNICODE_STRING DeviceName OPTIONAL,
    IN DEVICE_TYPE DeviceType,
    IN ULONG DeviceCharacteristics,
    IN BOOLEAN Exclusive,
    OUT PDEVICE_OBJECT *DeviceObject
)
{
    return IoCreateDevice(
        DriverObject,
        DeviceExtensionSize,
        DeviceName,
        DeviceType,
        DeviceCharacteristics,
        Exclusive,
        DeviceObject
    );
}

/**
 * IopDeleteDevice - Internal device deletion (alias)
 */
VOID IopDeleteDevice(
    IN PDEVICE_OBJECT DeviceObject
)
{
    IoDeleteDevice(DeviceObject);
}
