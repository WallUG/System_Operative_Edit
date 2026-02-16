# Bootloader - FreeLoader Adaptado de ReactOS

## Descripción

Este directorio contendrá el bootloader del sistema operativo, basado en FreeLoader de ReactOS.

## FreeLoader de ReactOS

FreeLoader es el bootloader de ReactOS, diseñado para:
- Cargar el kernel del sistema operativo
- Inicializar el hardware básico
- Configurar el modo de video
- Cargar drivers y módulos iniciales
- Transferir el control al kernel

### Características de FreeLoader
- Soporte para múltiples sistemas de archivos (FAT, NTFS, ext2/3)
- Arranque desde disco duro, CD/DVD, USB
- Soporte para multiboot
- Interfaz de usuario en modo texto
- Configuración flexible mediante archivos de configuración

## Proceso de Arranque

### 1. BIOS/UEFI
- El BIOS/UEFI carga el sector de arranque (MBR o GPT)
- El sector de arranque carga la primera etapa del bootloader

### 2. Primera Etapa del Bootloader
- Cargada desde el sector de arranque
- Inicializa el sistema de archivos
- Carga la segunda etapa del bootloader

### 3. Segunda Etapa del Bootloader (FreeLoader)
- Muestra el menú de arranque
- Lee el archivo de configuración
- Inicializa el hardware básico
- Carga el kernel en memoria
- Carga los drivers necesarios
- Configura los parámetros de arranque

### 4. Transferencia al Kernel
- FreeLoader transfiere el control al punto de entrada del kernel
- Pasa información sobre el hardware y configuración
- El kernel toma el control del sistema

## Plan de Integración

### Fase 1: Bootloader Simple
- Implementar un bootloader básico en Assembly
- Cargar el kernel desde el disco
- Transferir control al kernel

### Fase 2: Adaptar FreeLoader
- Extraer FreeLoader del código de ReactOS
- Adaptar para nuestro kernel
- Mantener compatibilidad con la estructura

### Fase 3: Configuración Avanzada
- Implementar menú de arranque
- Soporte para múltiples configuraciones
- Opciones de depuración

## Archivos Futuros

```
/boot
├── stage1.asm          # Primera etapa (sector de arranque)
├── stage2.asm          # Segunda etapa
├── freeldr/            # FreeLoader adaptado
│   ├── freeldr.c       # Código principal de FreeLoader
│   ├── fs/             # Soporte de sistemas de archivos
│   ├── arch/           # Código específico de arquitectura
│   └── include/        # Headers
├── config/             # Archivos de configuración
│   └── freeldr.ini     # Configuración del bootloader
└── README.md           # Este archivo
```

## Dependencias

- NASM (Netwide Assembler) para código Assembly
- GCC para código C del bootloader
- Herramientas de creación de imágenes (dd, mkisofs)

## Referencias

- [FreeLoader en ReactOS](https://github.com/reactos/reactos/tree/master/boot/freeldr)
- [Documentación de FreeLoader](https://reactos.org/wiki/FreeLoader)
- [OSDev Wiki - Bootloaders](https://wiki.osdev.org/Bootloader)

## Estado Actual

**Pendiente de implementación**

- [ ] Bootloader básico en Assembly
- [ ] Integración de FreeLoader
- [ ] Sistema de archivos de arranque
- [ ] Configuración de arranque

## Notas

- El bootloader debe ser lo más simple y robusto posible
- Se priorizará la compatibilidad con hardware común
- La integración con FreeLoader se hará gradualmente
