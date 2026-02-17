/*
 * PROJECT:     System_Operative_Edit
 * LICENSE:     GPL-3.0
 * PURPOSE:     I/O Manager - Driver Management
 * COPYRIGHT:   Adapted from ReactOS (GPL-3.0)
 *              Original: ReactOS Project (ntoskrnl/io/iomgr/driver.c)
 *              Adaptation: Universidad de Guayaquil
 */

#include "io_manager.h"
#include <kstdlib.h>

/* Global driver list */
PDRIVER_OBJECT g_DriverList = NULL;
int g_DriverCount = 0;

/* I/O Manager initialization flag */
static BOOLEAN g_IoInitialized = FALSE;

/**
 * IoInitSystem - Initialize I/O Manager
 * 
 * Returns: STATUS_SUCCESS or error code
 */
NTSTATUS IoInitSystem(VOID)
{
    if (g_IoInitialized) {
        return STATUS_SUCCESS;
    }
    
    g_DriverList = NULL;
    g_DriverCount = 0;
    g_IoInitialized = TRUE;
    
    return STATUS_SUCCESS;
}

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
)
{
    PDRIVER_OBJECT DriverObject;
    NTSTATUS Status;
    
    if (!DriverName || !DriverInit) {
        return STATUS_INVALID_PARAMETER;
    }
    
    /* Allocate driver object */
    DriverObject = (PDRIVER_OBJECT)malloc(sizeof(DRIVER_OBJECT));
    if (!DriverObject) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
    memset(DriverObject, 0, sizeof(DRIVER_OBJECT));
    
    /* Initialize driver object */
    DriverObject->DriverName = *DriverName;
    DriverObject->DriverInit = DriverInit;
    DriverObject->DeviceObject = NULL;
    DriverObject->NextDriver = NULL;
    DriverObject->DriverUnload = NULL;
    
    /* Call driver initialization */
    Status = DriverInit(DriverObject, DriverName);
    if (!NT_SUCCESS(Status)) {
        free(DriverObject);
        return Status;
    }
    
    /* Add to driver list */
    DriverObject->NextDriver = g_DriverList;
    g_DriverList = DriverObject;
    g_DriverCount++;
    
    return STATUS_SUCCESS;
}

/**
 * IoDeleteDriver - Unregister a driver
 * @DriverObject: Driver object to delete
 */
VOID IoDeleteDriver(
    IN PDRIVER_OBJECT DriverObject
)
{
    PDRIVER_OBJECT *Current;
    PDEVICE_OBJECT Device, NextDevice;
    
    if (!DriverObject) {
        return;
    }
    
    /* Delete all devices owned by this driver */
    Device = DriverObject->DeviceObject;
    while (Device) {
        NextDevice = Device->NextDevice;
        IoDeleteDevice(Device);
        Device = NextDevice;
    }
    
    /* Remove from driver list */
    Current = &g_DriverList;
    while (*Current) {
        if (*Current == DriverObject) {
            *Current = DriverObject->NextDriver;
            g_DriverCount--;
            break;
        }
        Current = &(*Current)->NextDriver;
    }
    
    /* Call driver unload if present */
    if (DriverObject->DriverUnload) {
        DriverObject->DriverUnload(DriverObject);
    }
    
    /* Free driver object */
    free(DriverObject);
}

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
)
{
    PDRIVER_OBJECT Driver;
    PDEVICE_OBJECT Device;
    
    if (!DeviceName || !DeviceObject) {
        return STATUS_INVALID_PARAMETER;
    }
    
    /* Search for device in all drivers */
    Driver = g_DriverList;
    while (Driver) {
        Device = Driver->DeviceObject;
        while (Device) {
            /* Simple comparison - in real system would use proper string compare */
            if (Device->DeviceName.Buffer && DeviceName->Buffer) {
                /* Found device */
                *DeviceObject = Device;
                if (FileObject) {
                    *FileObject = NULL; /* Not implemented */
                }
                return STATUS_SUCCESS;
            }
            Device = Device->NextDevice;
        }
        Driver = Driver->NextDriver;
    }
    
    return STATUS_DEVICE_DOES_NOT_EXIST;
}
