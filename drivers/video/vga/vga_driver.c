/*
 * PROJECT:     System_Operative_Edit
 * LICENSE:     GPL-3.0
 * PURPOSE:     VGA Display Driver - Main Entry
 * COPYRIGHT:   Adapted from ReactOS VGA Driver (GPL-3.0)
 *              Original: ReactOS Project (win32ss/drivers/displays/vga/main/enable.c)
 *              Adaptation: Universidad de Guayaquil
 */

#include "vga.h"

/* External functions */
extern NTSTATUS VgaInitializeDevice(PVGA_DEVICE_EXTENSION DevExt);
extern VOID VgaSetDeviceObject(PDEVICE_OBJECT DeviceObject);

/**
 * VgaDriverUnload - Driver unload routine
 * @DriverObject: Driver object
 */
static VOID VgaDriverUnload(
    IN PDRIVER_OBJECT DriverObject
)
{
    /* Clean up devices */
    PDEVICE_OBJECT Device = DriverObject->DeviceObject;
    while (Device) {
        PDEVICE_OBJECT Next = Device->NextDevice;
        IoDeleteDevice(Device);
        Device = Next;
    }
}

/**
 * VgaDriverEntry - VGA driver initialization
 * @DriverObject: Driver object
 * @RegistryPath: Registry path
 * 
 * Returns: STATUS_SUCCESS or error code
 */
NTSTATUS VgaDriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
)
{
    UNICODE_STRING DeviceName;
    PDEVICE_OBJECT DeviceObject;
    PVGA_DEVICE_EXTENSION DevExt;
    NTSTATUS Status;
    
    (VOID)RegistryPath;  /* Unused parameter */
    
    /* Create device name */
    /* Note: Using L prefix for wide string literal */
    static const WCHAR deviceNameStr[] = {'\\', 'D', 'e', 'v', 'i', 'c', 'e', '\\', 'V', 'G', 'A', 0};
    DeviceName.Buffer = (PWCHAR)deviceNameStr;
    DeviceName.Length = 10 * sizeof(WCHAR);
    DeviceName.MaximumLength = 11 * sizeof(WCHAR);
    
    /* Create device */
    Status = IoCreateDevice(
        DriverObject,
        sizeof(VGA_DEVICE_EXTENSION),
        &DeviceName,
        FILE_DEVICE_VIDEO,
        0,
        FALSE,
        &DeviceObject
    );
    
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    
    /* Get device extension */
    DevExt = (PVGA_DEVICE_EXTENSION)DeviceObject->DeviceExtension;
    
    /* Initialize VGA device */
    Status = VgaInitializeDevice(DevExt);
    if (!NT_SUCCESS(Status)) {
        IoDeleteDevice(DeviceObject);
        return Status;
    }
    
    /* Set up driver object */
    DriverObject->DriverUnload = VgaDriverUnload;
    
    /* Save device object for global access */
    VgaSetDeviceObject(DeviceObject);
    
    return STATUS_SUCCESS;
}
