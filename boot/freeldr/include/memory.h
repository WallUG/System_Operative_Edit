/*
 * FreeLoader - Simplified Boot Loader
 * Copyright (c) 2024 System_Operative_Edit Project
 * 
 * Este archivo es parte del proyecto System_Operative_Edit
 * Licencia: GPL-3.0
 * 
 * memory.h - Gestión de memoria
 */

#ifndef _MEMORY_H
#define _MEMORY_H

#include "freeldr.h"

/* Estructura de entrada del mapa de memoria E820 */
typedef struct {
    u64 base;       // Dirección base
    u64 length;     // Longitud del bloque
    u32 type;       // Tipo de memoria
    u32 acpi;       // Atributos ACPI extendidos
} __attribute__((packed)) E820Entry;

/* Tipos de memoria E820 */
#define E820_RAM        1   // Memoria usable
#define E820_RESERVED   2   // Memoria reservada
#define E820_ACPI       3   // Datos ACPI recuperables
#define E820_NVS        4   // Memoria NVS ACPI
#define E820_UNUSABLE   5   // Memoria defectuosa

/* Información de memoria del sistema */
typedef struct {
    u32 lower_memory;      // Memoria baja (KB)
    u32 upper_memory;      // Memoria alta (KB)
    u32 total_memory;      // Memoria total (KB)
    u32 available_memory;  // Memoria disponible (KB)
    u32 entry_count;       // Número de entradas E820
    E820Entry entries[32]; // Mapa de memoria E820
} MemoryInfo;

/* Funciones de memoria */
void MemoryInit(void);
void MemoryGetMap(void);
u32 MemoryGetTotal(void);
u32 MemoryGetLower(void);
u32 MemoryGetUpper(void);
void MemoryPrintMap(void);

#endif /* _MEMORY_H */
