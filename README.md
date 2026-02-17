# System Operative Edit

Sistema operativo basado en ReactOS para edición personalizada.

**Edición Universidad de Guayaquil** - Ahora con animación de arranque profesional en modo gráfico VGA featuring el logo institucional UG.

## ✨ Features

### Boot Animation
- **Modo Gráfico VGA 13h** (320×200, 256 colores)
- Logo de Universidad de Guayaquil con efectos fade in/out
- Barra de progreso animada
- **Fallback automático** a modo texto ASCII si VGA falla
- Paleta de colores institucionales

Ver [docs/BOOT_ANIMATION_GRAPHICS.md](docs/BOOT_ANIMATION_GRAPHICS.md) para detalles técnicos.

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

## Estructura del proyecto

- `/boot` - Bootloader y código de arranque
  - `/bootvid` - Driver de video VGA para boot
  - `/bootdata` - Assets y datos de boot (logos, etc.)
  - `/freeldr` - FreeLoader (segundo stage bootloader)
    - `/arch/i386` - Código específico de arquitectura
    - `/ui` - UI y animaciones gráficas
- `/kernel` - Código del kernel
  - `/hal` - Hardware Abstraction Layer
  - `/mm` - Memory Manager
  - `/io` - I/O Manager
  - `/ps` - Process Manager
- `/drivers` - Drivers de dispositivos
- `/include` - Headers compartidos
- `/docs` - Documentación técnica
- `/tools` - Herramientas de desarrollo
- `/scripts` - Scripts de build y test

## Testing

El sistema ahora incluye una animación de arranque profesional con el logo de Universidad de Guayaquil en modo gráfico VGA.

### Bootloader - Modo Gráfico (Primario)
Al arrancar con soporte VGA, verás:
- **Modo VGA 13h** (320×200, 256 colores)
- Logo "UG" renderizado con geometría en colores institucionales
- Efecto **fade-in** suave usando manipulación de paleta
- Barra de progreso gráfica verde con 5 etapas
- Branding "UNIVERSIDAD DE GUAYAQUIL" con líneas decorativas
- Efecto **fade-out** antes de pasar al kernel
- Duración: ~4.6 segundos

### Bootloader - Modo Texto (Fallback)
Si VGA no está disponible, verás:
- Logo "UG" en ASCII art con colores VGA de texto
- Barra de progreso con caracteres `=`, `>`, `-`
- Branding institucional
- Duración: ~3.5 segundos

### Documentación Completa
- [Boot Animation Graphics](docs/BOOT_ANIMATION_GRAPHICS.md) - Modo gráfico VGA
- [Boot Animation Text](boot/freeldr/BOOT_ANIMATION.md) - Modo texto fallback

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
