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
 * RtlCompareUnicodeString - Compare two UNICODE_STRING structures
 * @String1: First string
 * @String2: Second string
 * @CaseInSensitive: TRUE for case-insensitive comparison
 * 
 * Returns: 0 if equal, <0 if String1 < String2, >0 if String1 > String2
 */
static int RtlCompareUnicodeString(
    PCUNICODE_STRING String1,
    PCUNICODE_STRING String2,
    BOOLEAN CaseInSensitive
)
{
    size_t len1, len2, minlen, i;
    WCHAR c1, c2;
    
    (VOID)CaseInSensitive; /* Not implemented yet */
    
    if (!String1 || !String2) {
        return (String1 == String2) ? 0 : (String1 ? 1 : -1);
    }
    
    if (!String1->Buffer || !String2->Buffer) {
        return (String1->Buffer == String2->Buffer) ? 0 : (String1->Buffer ? 1 : -1);
    }
    
    len1 = String1->Length / sizeof(WCHAR);
    len2 = String2->Length / sizeof(WCHAR);
    minlen = (len1 < len2) ? len1 : len2;
    
    /* Compare character by character */
    for (i = 0; i < minlen; i++) {
        c1 = String1->Buffer[i];
        c2 = String2->Buffer[i];
        
        if (c1 != c2) {
            return (c1 < c2) ? -1 : 1;
        }
    }
    
    /* If all compared characters match, the shorter string is "less" */
    if (len1 == len2) {
        return 0;
    }
    return (len1 < len2) ? -1 : 1;
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
    
    (VOID)DesiredAccess; /* Not used in this implementation */
    
    if (!DeviceName || !DeviceObject) {
        return STATUS_INVALID_PARAMETER;
    }
    
    /* Search for device in all drivers */
    Driver = g_DriverList;
    while (Driver) {
        Device = Driver->DeviceObject;
        while (Device) {
            /* Compare device names */
            if (Device->DeviceName.Buffer && DeviceName->Buffer) {
                if (RtlCompareUnicodeString(&Device->DeviceName, DeviceName, FALSE) == 0) {
                    /* Found matching device */
                    *DeviceObject = Device;
                    if (FileObject) {
                        *FileObject = NULL; /* File objects not implemented yet */
                    }
                    return STATUS_SUCCESS;
                }
            }
            Device = Device->NextDevice;
        }
        Driver = Driver->NextDriver;
    }
    
    return STATUS_DEVICE_DOES_NOT_EXIST;
}
