# Herramientas de Compilación y Desarrollo

## Descripción

Este directorio contendrá herramientas y scripts útiles para el desarrollo, compilación y depuración del sistema operativo.

## Herramientas Planificadas

### 1. Scripts de Compilación

#### build.sh
Script principal de compilación que automatiza todo el proceso.

**Características**:
- Compila bootloader, kernel, drivers
- Genera imagen ISO booteable
- Limpieza de archivos temporales

#### cross-compiler.sh
Script para configurar un cross-compiler (gcc, binutils) para desarrollo de OS.

**Funciones**:
- Descargar código fuente de gcc y binutils
- Compilar para target i686-elf
- Instalar en directorio local

### 2. Scripts de Testing

#### run-qemu.sh
Ejecutar el sistema en QEMU con diferentes configuraciones.

**Opciones**:
- Con/sin GUI
- Diferentes cantidades de RAM
- Con/sin depuración (GDB)
- Redirección de puerto serial

#### run-bochs.sh
Ejecutar en Bochs para depuración detallada.

#### test-build.sh
Script de CI para verificar que todo compila correctamente.

### 3. Herramientas de Desarrollo

#### mkiso.sh
Crear imagen ISO booteable desde binarios compilados.

**Proceso**:
1. Crear estructura de directorios ISO
2. Copiar bootloader
3. Copiar kernel y drivers
4. Generar imagen con mkisofs/genisoimage

#### mount-iso.sh
Montar imagen ISO para inspección.

#### debug-qemu.sh
Lanzar QEMU con servidor GDB para depuración remota.

### 4. Análisis y Documentación

#### generate-docs.sh
Generar documentación del código con Doxygen.

#### check-style.sh
Verificar estilo de código (indent, spacing, etc.).

#### analyze-code.sh
Análisis estático con herramientas como cppcheck.

### 5. Utilidades

#### disk-image.sh
Crear imagen de disco con sistema de archivos.

**Características**:
- Crear imagen FAT16/FAT32
- Copiar archivos al filesystem
- Hacer booteable

#### extract-reactos.sh
Script para extraer componentes específicos de ReactOS.

**Funciones**:
- Descargar ReactOS source
- Extraer módulos específicos (FreeLoader, HAL, drivers)
- Preparar para integración

## Estructura Futura

```
/tools
├── build/
│   ├── build.sh              # Script principal de compilación
│   ├── cross-compiler.sh     # Configurar cross-compiler
│   └── clean.sh              # Limpiar archivos de compilación
├── test/
│   ├── run-qemu.sh           # Ejecutar en QEMU
│   ├── run-bochs.sh          # Ejecutar en Bochs
│   ├── run-virtualbox.sh     # Ejecutar en VirtualBox
│   └── test-build.sh         # CI build test
├── image/
│   ├── mkiso.sh              # Crear ISO
│   ├── mount-iso.sh          # Montar ISO
│   └── disk-image.sh         # Crear imagen de disco
├── debug/
│   ├── debug-qemu.sh         # Debug con QEMU+GDB
│   ├── debug-bochs.sh        # Debug con Bochs
│   └── serial-debug.sh       # Capturar salida serial
├── dev/
│   ├── generate-docs.sh      # Generar documentación
│   ├── check-style.sh        # Verificar estilo
│   ├── analyze-code.sh       # Análisis estático
│   └── extract-reactos.sh    # Extraer componentes de ReactOS
└── README.md                 # Este archivo
```

## Ejemplos de Uso

### Compilar Todo
```bash
./tools/build/build.sh
```

### Crear Imagen ISO
```bash
./tools/image/mkiso.sh
```

### Ejecutar en QEMU
```bash
./tools/test/run-qemu.sh
```

### Ejecutar con Depuración
```bash
# Terminal 1
./tools/debug/debug-qemu.sh

# Terminal 2
gdb build/kernel.bin
(gdb) target remote localhost:1234
(gdb) break kernel_main
(gdb) continue
```

### Configurar Cross-Compiler
```bash
./tools/build/cross-compiler.sh --target=i686-elf --prefix=/opt/cross
```

## Dependencias

### Para Compilación
- gcc o cross-compiler (i686-elf-gcc)
- binutils (ld, as)
- nasm
- make

### Para Creación de Imágenes
- mkisofs o genisoimage
- mtools (para manipular FAT)
- dd

### Para Testing
- qemu-system-i386
- bochs (opcional)
- virtualbox (opcional)

### Para Desarrollo
- gdb
- doxygen
- cppcheck
- indent o clang-format

## Instalación de Dependencias

### Ubuntu/Debian
```bash
sudo apt-get install build-essential nasm gcc make \
  genisoimage mtools qemu-system-x86 gdb doxygen cppcheck
```

### Fedora/RHEL
```bash
sudo dnf install gcc nasm make genisoimage mtools \
  qemu-system-x86 gdb doxygen cppcheck
```

### macOS
```bash
brew install nasm gcc make cdrtools qemu gdb doxygen cppcheck
```

### Windows
- Instalar MinGW-w64
- Instalar NASM
- Instalar QEMU for Windows
- Instalar Make for Windows

## Configuración

### Variables de Entorno

Algunas herramientas pueden usar estas variables:

```bash
export CROSS_COMPILE=i686-elf-  # Prefijo del cross-compiler
export QEMU_AUDIO_DRV=none       # Desactivar audio en QEMU
export ARCH=i386                 # Arquitectura objetivo
```

### Archivo de Configuración

Crear `tools/config.sh` con configuraciones personales:

```bash
# Configuración de herramientas
CROSS_COMPILE="i686-elf-"
QEMU_BIN="/usr/bin/qemu-system-i386"
BOCHS_BIN="/usr/bin/bochs"
BUILD_DIR="../build"
ISO_DIR="../iso"
```

## Integración Continua

### GitHub Actions

Ejemplo de workflow para CI:

```yaml
name: Build OS
on: [push, pull_request]
jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Install dependencies
        run: |
          sudo apt-get update
          sudo apt-get install build-essential nasm gcc make genisoimage
      - name: Build
        run: make all
      - name: Test build
        run: ./tools/test/test-build.sh
```

## Estado Actual

**Pendiente de implementación**

- [ ] Scripts de compilación
- [ ] Scripts de testing
- [ ] Scripts de creación de imágenes
- [ ] Herramientas de depuración
- [ ] Cross-compiler setup

## Referencias

- [OSDev - Building a Cross-Compiler](https://wiki.osdev.org/GCC_Cross-Compiler)
- [QEMU Documentation](https://www.qemu.org/docs/master/)
- [Bochs User Manual](http://bochs.sourceforge.net/doc/docbook/user/)
- [GDB Documentation](https://www.gnu.org/software/gdb/documentation/)

## Notas

- Todos los scripts deben ser POSIX-compliant cuando sea posible
- Incluir mensajes de error claros
- Documentar opciones y parámetros
- Hacer scripts idempotentes (safe to run multiple times)
