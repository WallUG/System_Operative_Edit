# System Operative Edit

Sistema operativo basado en ReactOS para edición personalizada.

**Edición Universidad de Guayaquil** - Ahora con animación de arranque profesional featuring el logo institucional UG. Ver [BOOT_ANIMATION_IMPLEMENTATION.md](BOOT_ANIMATION_IMPLEMENTATION.md) para detalles.

## Requisitos

### Herramientas de compilación:
- GCC cross-compiler (i686-elf-gcc) o gcc con soporte -m32
- NASM (Netwide Assembler)
- CMake >= 3.10
- Make
- GRUB tools (grub-mkrescue)
- xorriso

### Para testing:
- QEMU (qemu-system-i386)

### Instalación de dependencias (Ubuntu/Debian):
```bash
sudo apt-get update
sudo apt-get install build-essential nasm cmake qemu-system-x86 \
                     grub-pc-bin xorriso mtools gcc-multilib g++-multilib
```

## Compilación

### 1. Compilar el sistema:
```bash
./scripts/build.sh
```

### 2. Ejecutar en QEMU:
```bash
./scripts/run-qemu.sh
```

### 3. Crear ISO booteable:
```bash
./scripts/create-iso.sh
```

## 🤖 Compilación Automática (CI/CD)

Este proyecto incluye GitHub Actions para compilación automática.

### Descargar ISO Pre-compilado

1. Ve a [Actions](../../actions)
2. Click en el workflow "Build System Operative Edit" más reciente exitoso
3. Descarga el artifact "system-operative-edit-iso"
4. Extrae el ZIP y obtén `system_operative_edit.iso`

### Status del Build

![Build Status](https://github.com/WallUG/System_Operative_Edit/workflows/Build%20System%20Operative%20Edit/badge.svg)

Cada push a `main` compila automáticamente el sistema y genera un ISO descargable.

Ver [docs/CI_CD.md](docs/CI_CD.md) para más detalles.

## Estructura del proyecto

- `/boot` - Bootloader y código de arranque
- `/kernel` - Código del kernel
  - `/hal` - Hardware Abstraction Layer
  - `/mm` - Memory Manager
  - `/io` - I/O Manager
  - `/ps` - Process Manager
- `/drivers` - Drivers de dispositivos
- `/include` - Headers compartidos
- `/tools` - Herramientas de desarrollo
- `/scripts` - Scripts de build y test

## Testing

El sistema ahora incluye una animación de arranque profesional con el logo de Universidad de Guayaquil.

### Bootloader
Al arrancar, verás:
- Logo "UG" en ASCII art con colores institucionales
- Barra de progreso animada con 5 etapas
- Branding "UNIVERSIDAD DE GUAYAQUIL"
- Transiciones suaves durante ~3.5 segundos

### Kernel
El kernel mostrará un mensaje de bienvenida con branding institucional:
```
================================================================================
                       UNIVERSIDAD DE GUAYAQUIL
                    System Operative Edit v0.1
                    Edicion Universidad de Guayaquil
                         Based on ReactOS
================================================================================
Kernel initialized successfully!
```

Ver documentación completa en [boot/freeldr/BOOT_ANIMATION.md](boot/freeldr/BOOT_ANIMATION.md).

## Próximos pasos

- [ ] Implementar gestión de memoria completa
- [ ] Agregar manejo de interrupciones
- [ ] Implementar scheduler de procesos
- [ ] Agregar drivers básicos (teclado, mouse)
- [ ] Implementar sistema de archivos
