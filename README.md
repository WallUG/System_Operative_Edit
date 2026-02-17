# System Operative Edit

Sistema operativo basado en ReactOS para edición personalizada.

**Edición Universidad de Guayaquil** - Ahora con animación de arranque profesional en modo gráfico VGA featuring el logo institucional UG.

## ✨ Features

### Driver Framework (Fase 1)
- **I/O Manager** - Sistema completo de registro y gestión de drivers (adaptado de ReactOS)
  - Registro y descarga de drivers
  - Gestión de objetos DEVICE_OBJECT y DRIVER_OBJECT
  - Sistema básico de IRPs (I/O Request Packets)
- **Driver VGA** - Driver de video VGA completo
  - Modo texto 80×25 (compatibilidad actual)
  - Modo gráfico VGA 640×480×16 colores
  - Funciones de dibujado: pixel, líneas, rectángulos
  - Gestión de paleta de colores de 16 bits
- **HAL Display Support** - Abstracción de hardware para display

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
- `/drivers` - Drivers de dispositivos
  - `/framework` - I/O Manager framework (adaptado de ReactOS)
    - `io_manager.c` - Registro y gestión de drivers
    - `device.c` - Gestión de objetos de dispositivo
  - `/video/vga` - Driver VGA completo
    - `vga_driver.c` - Driver principal
    - `vga_init.c` - Inicialización VGA
    - `vga_screen.c` - Operaciones de pantalla
    - `vga_operations.c` - Operaciones de dibujado
    - `vga_hardware.c` - Acceso a hardware VGA
  - `/hal` - HAL display support
- `/include` - Headers compartidos
  - `/drivers/ddk` - Driver Development Kit headers
    - `wdm.h` - Windows Driver Model definitions
    - `ntddk.h` - NT DDK essentials
- `/lib` - Bibliotecas del kernel
  - `memory.c` - Gestión de memoria (malloc, free, memset, etc.)
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
El kernel mostrará un mensaje de bienvenida con branding institucional y luego:
1. Inicializa el HAL (Hardware Abstraction Layer)
2. Inicializa el I/O Manager
3. Carga el driver VGA en modo gráfico 640×480×16 colores
4. Inicializa el HAL Display
5. Dibuja un patrón de demostración con:
   - Barras de colores (16 colores VGA)
   - Rectángulos rellenos en colores primarios
   - Líneas horizontales y diagonales
   - Bordes alrededor de la pantalla

```
================================================================================
                       UNIVERSIDAD DE GUAYAQUIL
                    System Operative Edit v0.1
                    Edicion Universidad de Guayaquil
                         Based on ReactOS
================================================================================
Initializing HAL... OK
Initializing I/O Manager... OK
Loading VGA driver... OK
Initializing HAL Display... OK
Multiboot magic: Valid
Memory lower: Available
Memory upper: Available

Kernel initialized successfully!

Testing VGA graphics mode...
Drawing demo pattern in 5 seconds...
[Switches to VGA graphics mode with demo pattern]
```

Ver documentación completa en [boot/freeldr/BOOT_ANIMATION.md](boot/freeldr/BOOT_ANIMATION.md).

## Próximos pasos

### Driver Framework - Fase 2
- [ ] Implementar driver de teclado PS/2
- [ ] Implementar driver de mouse PS/2
- [ ] Agregar más modos de video VGA
- [ ] Implementar framebuffer manager
- [ ] Agregar soporte para texto en modo gráfico

### Kernel
- [ ] Implementar gestión de memoria completa
- [ ] Agregar manejo de interrupciones
- [ ] Implementar scheduler de procesos
- [ ] Implementar sistema de archivos

## Créditos

Este proyecto adapta componentes de ReactOS (GPL-3.0):
- **I/O Manager**: Adaptado de `ntoskrnl/io/iomgr/` de ReactOS
- **Driver VGA**: Adaptado de `win32ss/drivers/displays/vga/` de ReactOS
- **HAL Display**: Adaptado de `hal/halx86/generic/display.c` de ReactOS

Todos los archivos adaptados mantienen los créditos originales y respetan la licencia GPL-3.0.
