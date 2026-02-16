/*
 * FreeLoader - Simplified Boot Loader
 * Copyright (c) 2024 System_Operative_Edit Project
 * 
 * Este archivo es parte del proyecto System_Operative_Edit
 * Licencia: GPL-3.0
 * 
 * freeldr.h - Definiciones principales del FreeLoader
 */

#ifndef _FREELDR_H
#define _FREELDR_H

/* Tipos básicos */
typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;

typedef signed char        i8;
typedef signed short       i16;
typedef signed int         i32;
typedef signed long long   i64;

/* Definiciones de memoria */
#define FREELDR_BASE       0xF800   // Dirección base donde se carga FreeLoader
#define FREELDR_STACK      0x7BF0   // Tope de la pila
#define FREELDR_ENTRY      0xFA00   // Punto de entrada final

/* Definiciones de video */
#define VIDEO_BUFFER       0xB8000  // Buffer de video en modo texto
#define VIDEO_COLS         80       // Columnas en modo texto
#define VIDEO_ROWS         25       // Filas en modo texto

/* Definiciones de disco */
#define SECTOR_SIZE        512      // Tamaño de sector estándar

/* Códigos de retorno */
#define SUCCESS            0
#define ERROR              -1

/* Prototipos de funciones principales */
void BootMain(void) __attribute__((noreturn));
void Halt(void) __attribute__((noreturn));

/* Incluir otros headers */
#include "video.h"
#include "memory.h"
#include "disk.h"

#endif /* _FREELDR_H */
