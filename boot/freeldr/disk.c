/*
 * FreeLoader - Simplified Boot Loader
 * Copyright (c) 2024 System_Operative_Edit Project
 * 
 * Este archivo es parte del proyecto System_Operative_Edit
 * Licencia: GPL-3.0
 * 
 * disk.c - Funciones de acceso a disco
 * 
 * Implementa funciones básicas de lectura de disco usando INT 13h del BIOS
 * Soporta tanto modo CHS (Cylinder-Head-Sector) como LBA (Logical Block Addressing)
 */

#include "include/freeldr.h"
#include "include/disk.h"

/* Variable global con información del disco de arranque */
static DiskInfo boot_disk_info;
static u8 current_boot_drive = 0;

/*
 * DiskInit - Inicializa el subsistema de disco
 * @boot_drive: Número de unidad de arranque (de DL al arrancar)
 */
void DiskInit(u8 boot_drive)
{
    current_boot_drive = boot_drive;
    
    // Obtener información del disco
    DiskGetInfo(boot_drive, &boot_disk_info);
}

/*
 * DiskReset - Reinicia el controlador de disco
 * @drive: Número de unidad a reiniciar
 */
void DiskReset(u8 drive)
{
    u16 result;
    
    __asm__ volatile (
        "int $0x13\n"
        : "=a"(result)
        : "a"(0x0000), "d"(drive)
        : "cc"
    );
}

/*
 * DiskGetInfo - Obtiene información sobre un disco
 * @drive: Número de unidad
 * @info: Estructura donde se guardará la información
 * Retorna: 0 en éxito, -1 en error
 */
int DiskGetInfo(u8 drive, DiskInfo *info)
{
    u16 ax, cx, dx;
    
    info->drive = drive;
    
    // INT 13h, AH=08h: Obtener parámetros del disco
    __asm__ volatile (
        "push %%es\n"
        "int $0x13\n"
        "pop %%es\n"
        : "=a"(ax), "=c"(cx), "=d"(dx)
        : "a"(0x0800), "d"(drive)
        : "cc"
    );
    
    // Verificar si hubo error
    if (ax & 0xFF00) {
        return ERROR;
    }
    
    // Extraer información de los registros
    info->heads = ((dx >> 8) & 0xFF) + 1;           // DH: heads - 1
    info->sectors = cx & 0x3F;                       // CL bits 0-5: sectors per track
    info->cylinders = ((cx >> 8) & 0xFF) | ((cx & 0xC0) << 2); // CH + CL bits 6-7: cylinders
    
    // Calcular total de sectores
    info->total_sectors = (u32)info->cylinders * (u32)info->heads * (u32)info->sectors;
    
    return SUCCESS;
}

/*
 * DiskReadSectorsLBA - Lee sectores usando direccionamiento LBA
 * @drive: Número de unidad
 * @lba: Dirección LBA del primer sector
 * @count: Número de sectores a leer
 * @buffer: Buffer donde se guardarán los datos
 * Retorna: 0 en éxito, -1 en error
 */
static int DiskReadSectorsLBA(u8 drive, u32 lba, u16 count, void *buffer)
{
    // Paquete de direcciones de disco (DAP - Disk Address Packet)
    struct {
        u8 size;           // Tamaño del paquete (16)
        u8 reserved;       // Reservado (0)
        u16 count;         // Número de sectores a transferir
        u16 offset;        // Offset del buffer
        u16 segment;       // Segmento del buffer
        u32 lba_low;       // 32 bits bajos de LBA
        u32 lba_high;      // 32 bits altos de LBA
    } __attribute__((packed)) dap;
    
    dap.size = 16;
    dap.reserved = 0;
    dap.count = count;
    dap.offset = (u16)((u32)buffer & 0xFFFF);
    dap.segment = (u16)((u32)buffer >> 4);
    dap.lba_low = lba;
    dap.lba_high = 0;
    
    u16 result;
    
    // INT 13h, AH=42h: Lectura extendida
    __asm__ volatile (
        "int $0x13\n"
        : "=a"(result)
        : "a"(0x4200), "d"(drive), "S"(&dap)
        : "cc", "memory"
    );
    
    // Verificar carry flag (error)
    if (result & 0xFF00) {
        return ERROR;
    }
    
    return SUCCESS;
}

/*
 * DiskReadSectorsCHS - Lee sectores usando direccionamiento CHS
 * @drive: Número de unidad
 * @lba: Dirección LBA del primer sector (se convertirá a CHS)
 * @count: Número de sectores a leer
 * @buffer: Buffer donde se guardarán los datos
 * Retorna: 0 en éxito, -1 en error
 */
static int DiskReadSectorsCHS(u8 drive, u32 lba, u16 count, void *buffer)
{
    DiskInfo *info = &boot_disk_info;
    
    // Convertir LBA a CHS
    u16 cylinder = lba / (info->heads * info->sectors);
    u16 temp = lba % (info->heads * info->sectors);
    u16 head = temp / info->sectors;
    u16 sector = (temp % info->sectors) + 1;  // Los sectores empiezan en 1
    
    // Preparar registros para INT 13h, AH=02h
    u16 cx = ((cylinder & 0xFF) << 8) | ((cylinder & 0x300) >> 2) | (sector & 0x3F);
    u16 dx = (head << 8) | drive;
    u16 ax, result_ax;
    
    // Calcular segmento:offset del buffer
    u16 segment = (u16)((u32)buffer >> 4);
    u16 offset = (u16)((u32)buffer & 0xFFFF);
    
    // INT 13h, AH=02h: Leer sectores
    __asm__ volatile (
        "push %%es\n"
        "mov %4, %%es\n"
        "int $0x13\n"
        "pop %%es\n"
        : "=a"(result_ax)
        : "a"(0x0200 | (count & 0xFF)), "b"(offset), "c"(cx), "r"(segment), "d"(dx)
        : "cc", "memory"
    );
    
    // Verificar error
    if (result_ax & 0xFF00) {
        return ERROR;
    }
    
    return SUCCESS;
}

/*
 * DiskReadSectors - Lee sectores del disco
 * @drive: Número de unidad
 * @lba: Dirección LBA del primer sector
 * @count: Número de sectores a leer
 * @buffer: Buffer donde se guardarán los datos
 * Retorna: 0 en éxito, -1 en error
 * 
 * Intenta usar LBA primero, y si falla, usa CHS
 */
int DiskReadSectors(u8 drive, u32 lba, u16 count, void *buffer)
{
    int retry = 3;
    int result;
    
    while (retry > 0) {
        // Intentar con LBA primero
        result = DiskReadSectorsLBA(drive, lba, count, buffer);
        
        if (result == SUCCESS) {
            return SUCCESS;
        }
        
        // Si LBA falla, intentar con CHS
        result = DiskReadSectorsCHS(drive, lba, count, buffer);
        
        if (result == SUCCESS) {
            return SUCCESS;
        }
        
        // Reintentar después de reset
        DiskReset(drive);
        retry--;
    }
    
    return ERROR;
}
