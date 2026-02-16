/*
 * FreeLoader - Simplified Boot Loader
 * Copyright (c) 2024 System_Operative_Edit Project
 * 
 * Este archivo es parte del proyecto System_Operative_Edit
 * Licencia: GPL-3.0
 * 
 * memory.c - Gestión de memoria
 * 
 * Implementa funciones para detectar y gestionar la memoria del sistema
 * usando INT 15h del BIOS (función E820h para obtener mapa de memoria)
 */

#include "include/freeldr.h"
#include "include/memory.h"
#include "include/video.h"

/* Variable global con información de memoria */
static MemoryInfo mem_info;

/*
 * MemoryInit - Inicializa el subsistema de memoria
 * 
 * Llama a las funciones necesarias para detectar la memoria del sistema
 */
void MemoryInit(void)
{
    mem_info.lower_memory = 0;
    mem_info.upper_memory = 0;
    mem_info.total_memory = 0;
    mem_info.available_memory = 0;
    mem_info.entry_count = 0;
    
    // Obtener el mapa de memoria
    MemoryGetMap();
}

/*
 * MemoryGetMap - Obtiene el mapa de memoria usando INT 15h, E820h
 * 
 * Esta función usa el BIOS para obtener un mapa detallado de la memoria
 * del sistema, incluyendo regiones usables, reservadas, etc.
 */
void MemoryGetMap(void)
{
    u32 continuation = 0;
    u32 signature;
    u32 bytes;
    int entry = 0;
    
    // Usar inline assembly para llamar a INT 15h, E820h
    do {
        E820Entry *e = &mem_info.entries[entry];
        
        __asm__ volatile (
            "int $0x15\n"
            : "=a"(signature), "=c"(bytes), "=b"(continuation)
            : "a"(0xE820), "b"(continuation), "c"(24), "d"(0x534D4150), "D"(e)
            : "memory"
        );
        
        // Verificar si la llamada fue exitosa
        if (signature != 0x534D4150) {
            break;  // Error: firma incorrecta
        }
        
        // Si bytes es 0, hemos terminado
        if (bytes == 0) {
            break;
        }
        
        // Incrementar contador de entradas
        entry++;
        mem_info.entry_count = entry;
        
        // Si continuation es 0, hemos terminado
        if (continuation == 0) {
            break;
        }
        
        // Límite de seguridad
        if (entry >= 32) {
            break;
        }
        
    } while (continuation != 0);
    
    // Calcular memoria total y disponible
    for (int i = 0; i < mem_info.entry_count; i++) {
        E820Entry *e = &mem_info.entries[i];
        
        if (e->type == E820_RAM) {
            u32 size_kb = (u32)(e->length / 1024);
            mem_info.available_memory += size_kb;
            
            // Separar entre memoria baja (<1MB) y alta (>1MB)
            if (e->base < 0x100000) {
                // Memoria baja
                u32 low_size = (u32)(e->length / 1024);
                if (e->base + e->length > 0x100000) {
                    low_size = (u32)((0x100000 - e->base) / 1024);
                }
                mem_info.lower_memory += low_size;
            } else {
                // Memoria alta
                mem_info.upper_memory += size_kb;
            }
        }
    }
    
    mem_info.total_memory = mem_info.lower_memory + mem_info.upper_memory;
}

/*
 * MemoryGetTotal - Obtiene la memoria total del sistema en KB
 * Retorna: Memoria total en KB
 */
u32 MemoryGetTotal(void)
{
    return mem_info.total_memory;
}

/*
 * MemoryGetLower - Obtiene la memoria baja (<1MB) en KB
 * Retorna: Memoria baja en KB
 */
u32 MemoryGetLower(void)
{
    return mem_info.lower_memory;
}

/*
 * MemoryGetUpper - Obtiene la memoria alta (>1MB) en KB
 * Retorna: Memoria alta en KB
 */
u32 MemoryGetUpper(void)
{
    return mem_info.upper_memory;
}

/*
 * MemoryPrintMap - Imprime el mapa de memoria en pantalla
 * 
 * Función de depuración que muestra todas las regiones de memoria
 */
void MemoryPrintMap(void)
{
    const char *type_names[] = {
        "Unknown",
        "Available",
        "Reserved",
        "ACPI",
        "ACPI NVS",
        "Bad Memory"
    };
    
    VideoPutString("Mapa de Memoria (E820):\n");
    VideoPutString("========================\n\n");
    
    for (int i = 0; i < mem_info.entry_count; i++) {
        E820Entry *e = &mem_info.entries[i];
        
        // Imprimir dirección base (simplificado, solo parte baja)
        VideoPutString("  Base: ");
        VideoHexPrint((u32)e->base);
        
        VideoPutString("  Length: ");
        VideoHexPrint((u32)e->length);
        
        VideoPutString("  Type: ");
        if (e->type <= 5) {
            VideoPutString(type_names[e->type]);
        } else {
            VideoPutString("Unknown");
        }
        VideoPutString("\n");
    }
    
    VideoPutString("\nResumen:\n");
    VideoPutString("  Memoria baja:       ");
    VideoDecPrint(mem_info.lower_memory);
    VideoPutString(" KB\n");
    
    VideoPutString("  Memoria alta:       ");
    VideoDecPrint(mem_info.upper_memory);
    VideoPutString(" KB\n");
    
    VideoPutString("  Memoria total:      ");
    VideoDecPrint(mem_info.total_memory);
    VideoPutString(" KB\n");
    
    VideoPutString("  Memoria disponible: ");
    VideoDecPrint(mem_info.available_memory);
    VideoPutString(" KB\n");
}

/*
 * VideoHexPrint - Función auxiliar para imprimir números hexadecimales
 * @value: Valor a imprimir
 */
void VideoHexPrint(u32 value)
{
    const char hex_chars[] = "0123456789ABCDEF";
    char buffer[11];
    buffer[0] = '0';
    buffer[1] = 'x';
    
    for (int i = 7; i >= 0; i--) {
        buffer[2 + (7 - i)] = hex_chars[(value >> (i * 4)) & 0xF];
    }
    buffer[10] = '\0';
    
    VideoPutString(buffer);
}

/*
 * VideoDecPrint - Función auxiliar para imprimir números decimales
 * @value: Valor a imprimir
 */
void VideoDecPrint(u32 value)
{
    char buffer[12];
    int pos = 0;
    
    if (value == 0) {
        VideoPutChar('0');
        return;
    }
    
    // Extraer dígitos
    u32 temp = value;
    while (temp > 0) {
        buffer[pos++] = '0' + (temp % 10);
        temp /= 10;
    }
    
    // Imprimir en orden inverso
    for (int i = pos - 1; i >= 0; i--) {
        VideoPutChar(buffer[i]);
    }
}
