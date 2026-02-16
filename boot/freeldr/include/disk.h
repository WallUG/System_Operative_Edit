/*
 * FreeLoader - Simplified Boot Loader
 * Copyright (c) 2024 System_Operative_Edit Project
 * 
 * Este archivo es parte del proyecto System_Operative_Edit
 * Licencia: GPL-3.0
 * 
 * disk.h - Funciones de acceso a disco
 */

#ifndef _DISK_H
#define _DISK_H

#include "freeldr.h"

/* Estructura de parámetros de disco (INT 13h) */
typedef struct {
    u8 drive;           // Número de unidad
    u16 cylinders;      // Número de cilindros
    u8 heads;           // Número de cabezas
    u8 sectors;         // Sectores por pista
    u32 total_sectors;  // Total de sectores
} DiskInfo;

/* Funciones de disco */
void DiskInit(u8 boot_drive);
int DiskReadSectors(u8 drive, u32 lba, u16 count, void *buffer);
int DiskGetInfo(u8 drive, DiskInfo *info);
void DiskReset(u8 drive);

#endif /* _DISK_H */
