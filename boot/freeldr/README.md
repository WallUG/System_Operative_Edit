# FreeLoader - Simplified Boot Loader

FreeLoader es el segundo stage del bootloader del sistema operativo. Es cargado por el boot sector y se encarga de inicializar el hardware, detectar memoria y preparar el entorno para el kernel.

## Arquitectura

FreeLoader es un bootloader simplificado escrito en C (con algunas partes en Assembly inline) diseñado para ser:
- **Simple**: Código limpio y fácil de entender
- **Educativo**: Comentarios detallados en español
- **Funcional**: Capaz de arrancar y detectar hardware básico
- **Extensible**: Base sólida para futuras mejoras

## Estructura de Archivos

```
boot/freeldr/
├── freeldr.c         # Punto de entrada principal
├── memory.c          # Gestión de memoria (INT 15h, E820)
├── disk.c            # Acceso a disco (INT 13h, LBA/CHS)
├── video.c           # Video en modo texto (VGA)
├── string.c          # Funciones de strings (sin libc)
└── include/
    ├── freeldr.h     # Definiciones principales
    ├── memory.h      # Estructuras de memoria
    ├── disk.h        # Estructuras de disco
    └── video.h       # Funciones de video
```

## Componentes

### freeldr.c - Punto de Entrada Principal

- **Función**: `BootMain()`
- **Responsabilidades**:
  - Inicializar todos los subsistemas
  - Mostrar información del sistema
  - Preparar el entorno para el kernel
  - Transferir control al kernel (fase futura)

### video.c - Subsistema de Video

- **Modo**: Texto VGA 80x25
- **Acceso**: Directo a memoria de video (0xB8000)
- **Funciones**:
  - `VideoInit()` - Inicializa el video
  - `VideoClearScreen()` - Limpia la pantalla
  - `VideoPutChar()` - Escribe un carácter
  - `VideoPutString()` - Escribe una cadena
  - `VideoSetCursor()` - Posiciona el cursor
  - `VideoSetColor()` - Cambia el color del texto

### memory.c - Gestión de Memoria

- **Método**: INT 15h, función E820h (BIOS)
- **Características**:
  - Detecta toda la memoria del sistema
  - Crea un mapa de memoria detallado
  - Diferencia entre memoria usable y reservada
  - Separa memoria baja (<1MB) y alta (>1MB)
- **Funciones**:
  - `MemoryInit()` - Inicializa el subsistema
  - `MemoryGetMap()` - Obtiene el mapa de memoria
  - `MemoryGetTotal()` - Retorna memoria total
  - `MemoryPrintMap()` - Imprime el mapa (debug)

### disk.c - Acceso a Disco

- **Método**: INT 13h del BIOS
- **Modos soportados**:
  - LBA (Logical Block Addressing) - Preferido
  - CHS (Cylinder-Head-Sector) - Fallback
- **Funciones**:
  - `DiskInit()` - Inicializa el subsistema
  - `DiskReadSectors()` - Lee sectores del disco
  - `DiskGetInfo()` - Obtiene información del disco
  - `DiskReset()` - Reinicia el controlador

### string.c - Funciones de Strings

Implementación de funciones estándar de C sin usar libc:
- `strlen()` - Longitud de cadena
- `strcpy()` - Copiar cadena
- `strcmp()` - Comparar cadenas
- `memcpy()` - Copiar memoria
- `memset()` - Llenar memoria
- `memcmp()` - Comparar memoria

## Proceso de Arranque

1. **Boot Sector** carga FreeLoader en `0000:F800`
2. Boot sector salta a `_start` en FreeLoader
3. `_start` (Assembly):
   - Configura segmentos (DS, ES, SS)
   - Configura la pila en `0x7BF0`
   - Guarda parámetros de arranque (DL, DH)
   - Llama a `BootMain()`
4. `BootMain()` (C):
   - Inicializa video
   - Inicializa memoria
   - Inicializa disco
   - Muestra información del sistema
   - Prepara para cargar kernel (fase futura)

## Mapa de Memoria

```
0000:0000 - 0000:0FFF   Tabla de vectores de interrupción y datos BIOS
0000:1000 - 0000:6FFF   Área de pila en modo real
0000:7000 - 0000:7FFF   Línea de comandos (multiboot)
0000:7C00 - 0000:7DFF   Boot sector (cargado por BIOS)
0000:8000 - 0000:F7FF   Libre
0000:F800 - 0000:FFFF   FreeLoader (este programa)
0001:0000 - 9000:0000   Memoria disponible para el sistema
9000:0000 - 9000:FFFF   Buffer de lectura de disco
A000:0000 - B7FF:FFFF   Reservado
B800:0000 - B8FF:FFFF   Memoria de video
C000:0000 - FFFF:FFFF   BIOS y hardware
```

## Compilación

FreeLoader se compila con GCC para arquitectura i386 en modo freestanding:

```bash
gcc -m32 -ffreestanding -c freeldr.c -o freeldr.o
gcc -m32 -ffreestanding -c video.c -o video.o
gcc -m32 -ffreestanding -c memory.c -o memory.o
gcc -m32 -ffreestanding -c disk.c -o disk.o
gcc -m32 -ffreestanding -c string.c -o string.o

ld -m elf_i386 -T linker.ld -o freeldr.sys \
   freeldr.o video.o memory.o disk.o string.o
```

Ver `../Makefile` para el proceso completo.

## Flags de Compilación

- `-m32`: Generar código de 32 bits
- `-ffreestanding`: Sin biblioteca estándar
- `-nostdlib`: No vincular con bibliotecas estándar
- `-Wl,--oformat=binary`: Generar binario plano (no ELF)

## Licencia

Este código es original del proyecto System_Operative_Edit.
Licencia: GPL-3.0

## Estado Actual

**Fase 1 - COMPLETA**:
- ✅ Estructura básica del FreeLoader
- ✅ Inicialización de video
- ✅ Detección de memoria
- ✅ Acceso básico a disco
- ✅ Funciones de strings

**Fase 2 - PENDIENTE**:
- ⏳ Cargador de configuración
- ⏳ Búsqueda y carga del kernel
- ⏳ Preparación de estructuras para el kernel
- ⏳ Transferencia de control al kernel

## Extensiones Futuras

1. **Multiboot Support**: Soporte para especificación Multiboot
2. **EFI Support**: Soporte para UEFI (además de BIOS legacy)
3. **File Systems**: Soporte para leer FAT32, ext2, etc.
4. **Menu**: Interfaz de menú para seleccionar opciones de arranque
5. **Protected Mode**: Transición completa a modo protegido antes de cargar kernel
6. **64-bit**: Soporte para kernels de 64 bits (Long Mode)

## Referencias

- [OSDev Wiki](https://wiki.osdev.org) - Información sobre desarrollo de sistemas operativos
- [ReactOS FreeLoader](https://github.com/reactos/reactos) - Inspiración para el diseño
- Intel/AMD Manuals - Documentación de arquitectura x86
- BIOS Interrupts - Referencia de INT 10h, 13h, 15h

## Créditos

Código original: System_Operative_Edit Project
Inspirado por: ReactOS FreeLoader (diseño simplificado)
