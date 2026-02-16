# Bootloader

Sistema de arranque para System_Operative_Edit. Implementa un bootloader híbrido que combina:
- **Boot sectors** probados de ReactOS (GPL-2.0)
- **FreeLoader** simplificado desarrollado desde cero

## Arquitectura

El bootloader utiliza una arquitectura de dos etapas:

```
+----------------+     +------------------+     +--------------+
|  Boot Sector   | --> |   FreeLoader     | --> |    Kernel    |
|   (Stage 1)    |     |    (Stage 2)     |     |  (Stage 3)   |
+----------------+     +------------------+     +--------------+
   512/2048 bytes       ~8KB (expandible)        (futuro)
   Cargado por BIOS     Cargado por boot         Cargado por
                        sector                    FreeLoader
```

### Stage 1: Boot Sector

El boot sector es cargado por el BIOS en la dirección `0x7C00` y tiene las siguientes responsabilidades:

1. **Inicialización básica**: Configura segmentos y pila
2. **Búsqueda de FreeLoader**: Localiza el archivo `FREELDR.SYS` en el sistema de archivos
3. **Carga de FreeLoader**: Carga `FREELDR.SYS` en la dirección `0x0000:F800`
4. **Transferencia de control**: Salta a FreeLoader

Soportamos tres tipos de boot sectors:
- **FAT12/16** (`fat.S`): Para disquetes y discos duros con FAT
- **FAT32** (`fat32.S`): Para discos duros modernos con FAT32
- **ISO-9660** (`isoboot.S`): Para CDs/DVDs bootables

### Stage 2: FreeLoader

FreeLoader es el cargador principal que:

1. **Inicializa hardware**: Video, memoria, disco
2. **Detecta memoria**: Usa INT 15h (E820) para obtener mapa de memoria
3. **Configura entorno**: Prepara estructuras para el kernel
4. **Carga kernel**: Busca y carga el kernel en memoria (fase futura)
5. **Transfiere control**: Pasa control al kernel (fase futura)

## Estructura de Directorios

```
boot/
├── Makefile              # Sistema de compilación
├── README.md             # Este archivo
├── bootsect/             # Boot sectors (de ReactOS)
│   ├── README.md         # Documentación de boot sectors
│   ├── fat.S             # Boot sector FAT12/16
│   ├── fat32.S           # Boot sector FAT32
│   └── isoboot.S         # Boot sector ISO-9660
├── freeldr/              # FreeLoader (implementación propia)
│   ├── README.md         # Documentación de FreeLoader
│   ├── freeldr.c         # Punto de entrada principal
│   ├── video.c           # Funciones de video
│   ├── memory.c          # Gestión de memoria
│   ├── disk.c            # Acceso a disco
│   ├── string.c          # Funciones de strings
│   └── include/          # Headers
│       ├── freeldr.h     # Definiciones principales
│       ├── video.h       # API de video
│       ├── memory.h      # Estructuras de memoria
│       └── disk.h        # API de disco
├── docs/                 # Documentación adicional
│   └── freeldr_notes.txt # Notas técnicas de ReactOS
└── build/                # Archivos compilados (generado)
    ├── fat.bin           # Boot sector FAT compilado
    ├── fat32.bin         # Boot sector FAT32 compilado
    ├── isoboot.bin       # Boot sector ISO compilado
    └── freeldr.sys       # FreeLoader compilado
```

## Componentes

### De ReactOS (GPL-2.0)

Los siguientes componentes provienen del proyecto [ReactOS](https://reactos.org):

- **Boot sectors** (`bootsect/*.S`): Código Assembly de los boot sectors
  - Copyright: Brian Palmer y otros contribuidores de ReactOS
  - Licencia: GPL-2.0+ compatible
  - **NO MODIFICADO**: Estos archivos se mantienen tal cual de ReactOS

- **Documentación técnica** (`docs/freeldr_notes.txt`): Notas sobre FreeLoader
  - Fuente: ReactOS
  - Describe el proceso de arranque y layout de memoria

### Original (GPL-3.0)

Los siguientes componentes son desarrollo original del proyecto:

- **FreeLoader simplificado** (`freeldr/*`): Implementación en C
  - Copyright: System_Operative_Edit Project (2024)
  - Licencia: GPL-3.0
  - Código nuevo escrito desde cero

## Proceso de Arranque Completo

### 1. BIOS Power-On

```
BIOS ejecuta POST (Power-On Self Test)
     ↓
BIOS busca dispositivo bootable
     ↓
BIOS carga boot sector en 0x7C00
     ↓
BIOS salta a 0x7C00
```

### 2. Boot Sector (Stage 1)

```
Boot Sector (0x7C00):
  • Configura DS, ES, SS = 0
  • Configura stack en 0x7BF0
  • Lee FAT/directorio raíz
  • Busca FREELDR.SYS
  • Carga FREELDR.SYS en 0xF800
  • DL = drive number
  • DH = partition
  • Salta a 0xF800 (FreeLoader)
```

### 3. FreeLoader (Stage 2)

```
FreeLoader (0xF800):
  • Guarda parámetros (DL, DH)
  • Inicializa video (modo texto VGA)
  • Muestra banner
  • Detecta memoria (INT 15h E820)
  • Inicializa disco (INT 13h)
  • Muestra información del sistema
  • [FUTURO] Lee configuración
  • [FUTURO] Busca kernel
  • [FUTURO] Carga kernel
  • [FUTURO] Prepara estructuras
  • [FUTURO] Salta a kernel
```

### 4. Kernel (Stage 3) - FUTURO

```
Kernel:
  • Recibe información de FreeLoader
  • Configura modo protegido/largo
  • Inicializa subsistemas
  • Inicia sistema operativo
```

## Compilación

### Requisitos

- **Sistema operativo**: Linux o compatible (WSL en Windows)
- **Herramientas**:
  - GNU Binutils (`as`, `ld`)
  - GCC con soporte i386
  - Make

Instalar en Ubuntu/Debian:
```bash
sudo apt-get install build-essential gcc-multilib binutils
```

### Compilar Todo

```bash
cd boot
make all
```

Esto genera:
- `build/fat.bin` - Boot sector FAT12/16 (512 bytes)
- `build/fat32.bin` - Boot sector FAT32 (512 bytes)  
- `build/isoboot.bin` - Boot sector ISO-9660 (2048 bytes)
- `build/freeldr.sys` - FreeLoader ejecutable

### Compilar Solo Boot Sectors

```bash
make bootsect
```

### Compilar Solo FreeLoader

```bash
make freeldr
```

### Limpiar

```bash
make clean
```

### Ver Información

```bash
make info      # Muestra versiones de herramientas
make help      # Muestra ayuda de targets
```

## Mapa de Memoria

```
+-------------------+ 0x00000000
| IVT + BIOS Data   |  (4 KB)     Vectores de interrupción
+-------------------+ 0x00001000
| Real Mode Stack   |  (24 KB)    Pila en modo real
+-------------------+ 0x00007000
| Cmdline           |  (4 KB)     Línea de comandos (multiboot)
+-------------------+ 0x00007C00
| Boot Sector       |  (512 B)    Cargado por BIOS
+-------------------+ 0x00007E00
| Free              |  (30 KB)
+-------------------+ 0x0000F800
| FreeLoader        |  (2 KB)     Este programa
+-------------------+ 0x00010000
| Available RAM     |  ~640 KB    Memoria convencional
+-------------------+ 0x0009FC00
| Extended BIOS     |  (1 KB)
+-------------------+ 0x000A0000
| Video Memory      |  (128 KB)   VGA/EGA framebuffer
+-------------------+ 0x000C0000
| BIOS ROM          |  (256 KB)   System BIOS
+-------------------+ 0x00100000
| Extended Memory   |  (resto)    >1MB (Protected/Long mode)
+-------------------+
```

## Pruebas

### Probar en QEMU

```bash
# Crear imagen de disco de 32MB (apropiada para FAT12/16)
dd if=/dev/zero of=disk.img bs=1M count=32

# Escribir boot sector
dd if=build/fat.bin of=disk.img conv=notrunc

# Copiar FreeLoader al disco (necesita montar el filesystem)
# ... (pendiente: script de instalación)

# Ejecutar en QEMU
qemu-system-i386 -drive file=disk.img,format=raw
```

### Probar desde ISO

```bash
# Crear ISO booteable (requiere mkisofs/genisoimage)
mkdir -p iso_root/boot
cp build/freeldr.sys iso_root/boot/
mkisofs -o bootable.iso -b boot/isoboot.bin -no-emul-boot \
        -boot-load-size 4 iso_root/

# Ejecutar en QEMU
qemu-system-i386 -cdrom bootable.iso
```

## Créditos

### Boot Sectors
- **Proyecto**: ReactOS (https://reactos.org)
- **Autores principales**: Brian Palmer, H. Peter Anvin, y otros
- **Fuente**: https://github.com/reactos/reactos
- **Licencia**: GPL-2.0+

### FreeLoader Simplificado
- **Proyecto**: System_Operative_Edit
- **Autores**: Equipo System_Operative_Edit
- **Licencia**: GPL-3.0

### Agradecimientos
- Comunidad ReactOS por los excelentes boot sectors
- OSDev.org por la documentación
- Comunidad open source en general

## Estado del Proyecto

### Completado ✅

- [x] Boot sectors de ReactOS integrados
- [x] FreeLoader básico funcional
- [x] Inicialización de video (modo texto)
- [x] Detección de memoria (E820)
- [x] Funciones básicas de disco (INT 13h)
- [x] Sistema de compilación (Makefile)
- [x] Documentación completa

### En Desarrollo 🚧

- [ ] Cargador de configuración (boot.ini)
- [ ] Sistema de archivos (lectura de FAT)
- [ ] Cargador de kernel
- [ ] Transición a modo protegido
- [ ] Scripts de instalación

### Futuro 📅

- [ ] Soporte Multiboot
- [ ] Soporte EFI/UEFI
- [ ] Modo de 64 bits (Long Mode)
- [ ] Interfaz de menú interactiva
- [ ] Más sistemas de archivos (ext2, NTFS)

## Referencias

- [ReactOS](https://reactos.org) - Sistema operativo open source
- [OSDev Wiki](https://wiki.osdev.org) - Wiki de desarrollo de OS
- [Intel Manuals](https://software.intel.com/content/www/us/en/develop/articles/intel-sdm.html) - Manuales de arquitectura x86
- [BIOS Interrupts](https://en.wikipedia.org/wiki/BIOS_interrupt_call) - Referencia de interrupciones
- [El Torito](https://wiki.osdev.org/El-Torito) - Especificación de boot desde CD

## Licencia

Este proyecto combina código de diferentes fuentes:

- **Boot sectors** (`bootsect/`): GPL-2.0+ (de ReactOS)
- **FreeLoader** (`freeldr/`): GPL-3.0 (original)
- **Documentación**: GPL-3.0

Ver LICENSE en el directorio raíz para más detalles.

## Contribuir

Para contribuir al bootloader:

1. Los **boot sectors** NO deben modificarse (vienen de ReactOS)
2. Las mejoras al **FreeLoader** son bienvenidas
3. Seguir el estilo de código existente
4. Comentarios en español
5. Documentar cambios significativos

---

**System_Operative_Edit Project** - 2024
