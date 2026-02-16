# Drivers - Controladores de Hardware

## Descripción

Este directorio contendrá los controladores (drivers) de hardware del sistema operativo. Los drivers permiten al kernel interactuar con el hardware del sistema.

## Drivers Planificados

### Drivers Esenciales

#### 1. Driver de Teclado
- **Propósito**: Capturar entrada del teclado
- **Fuente**: Adaptado de ReactOS
- **Prioridad**: Alta
- **Características**:
  - Soporte para teclados PS/2
  - Soporte para teclados USB (futuro)
  - Mapeo de teclas
  - Soporte para diferentes layouts

#### 2. Driver de Video VGA
- **Propósito**: Salida de video en modo texto y gráfico
- **Fuente**: Implementación propia + ReactOS
- **Prioridad**: Alta
- **Características**:
  - Modo texto 80x25
  - Modos gráficos VGA
  - Double buffering
  - Soporte para diferentes resoluciones

#### 3. Driver de Disco (IDE/SATA)
- **Propósito**: Acceso a dispositivos de almacenamiento
- **Fuente**: Adaptado de ReactOS
- **Prioridad**: Alta
- **Características**:
  - Soporte IDE/ATA
  - Soporte SATA básico
  - DMA transfer
  - Detección de dispositivos

#### 4. Driver de Sistema de Archivos
- **Propósito**: Lectura y escritura de archivos
- **Fuente**: Adaptado de ReactOS
- **Prioridad**: Alta
- **Sistemas soportados**:
  - FAT16/FAT32
  - NTFS (lectura, futuro)
  - ext2 (futuro)

### Drivers Adicionales (Futuro)

#### 5. Driver de Red
- **Ethernet**: NE2000, RTL8139
- **Protocolo**: TCP/IP stack básico

#### 6. Driver de Audio
- **AC97**: Audio básico
- **SoundBlaster**: Compatibilidad legacy

#### 7. Driver de Mouse
- **PS/2**: Mouse básico
- **USB**: Mouse USB (futuro)

#### 8. Driver USB
- **USB Host Controller**: UHCI, OHCI, EHCI
- **USB Device Support**: Teclado, mouse, almacenamiento

## Drivers de ReactOS a Adaptar

ReactOS proporciona una extensa colección de drivers que pueden ser adaptados:

### Storage
- `drivers/storage/class/disk/` - Disk class driver
- `drivers/storage/ide/` - IDE driver
- `drivers/storage/floppy/` - Floppy driver

### Filesystems
- `drivers/filesystems/fastfat/` - FAT filesystem
- `drivers/filesystems/cdfs/` - CD filesystem
- `drivers/filesystems/ntfs/` - NTFS filesystem

### Input
- `drivers/input/keyboard/` - Keyboard drivers
- `drivers/input/mouse/` - Mouse drivers

### Video
- `drivers/video/displays/vga/` - VGA display driver
- `drivers/video/miniport/vga/` - VGA miniport driver

## Arquitectura de Drivers

### Modelo de Driver
```
Aplicación
    ↓
System Calls
    ↓
I/O Manager (Kernel)
    ↓
Driver Interface
    ↓
Driver Específico
    ↓
Hardware
```

### Estructura de un Driver

```c
// Estructura básica de un driver
typedef struct _DRIVER {
    char *name;
    int (*init)(void);
    int (*read)(void *buf, size_t count);
    int (*write)(const void *buf, size_t count);
    int (*ioctl)(int cmd, void *arg);
    void (*cleanup)(void);
} DRIVER;
```

### Inicialización de Drivers

1. **Detección de Hardware**: El kernel detecta el hardware presente
2. **Carga del Driver**: El driver correspondiente se carga en memoria
3. **Inicialización**: Se llama a la función `init()` del driver
4. **Registro**: El driver se registra con el I/O Manager
5. **Listo**: El driver está disponible para uso

## Estructura de Archivos Futura

```
/drivers
├── input/
│   ├── keyboard/
│   │   ├── ps2kbd.c
│   │   └── usbkbd.c
│   └── mouse/
│       ├── ps2mouse.c
│       └── usbmouse.c
├── storage/
│   ├── ide/
│   │   ├── ide.c
│   │   └── ide.h
│   ├── sata/
│   │   └── ahci.c
│   └── floppy/
│       └── floppy.c
├── video/
│   ├── vga/
│   │   ├── vga_text.c
│   │   ├── vga_gfx.c
│   │   └── vga.h
│   └── vesa/
│       └── vesa.c
├── filesystem/
│   ├── fat/
│   │   ├── fat.c
│   │   └── fat.h
│   └── ntfs/
│       └── ntfs.c
├── network/
│   ├── ne2000/
│   │   └── ne2000.c
│   └── rtl8139/
│       └── rtl8139.c
├── audio/
│   └── ac97/
│       └── ac97.c
└── README.md
```

## Guía de Desarrollo de Drivers

### 1. Estructura Básica

Todo driver debe implementar:
- Función de inicialización
- Funciones de lectura/escritura
- Función de limpieza
- Manejo de interrupciones (si aplica)

### 2. Registro de Driver

```c
int register_driver(DRIVER *drv) {
    // Registrar con el I/O Manager
    return io_register_driver(drv);
}
```

### 3. Manejo de Interrupciones

```c
void driver_irq_handler(int irq) {
    // Manejar interrupción
    // Leer datos del hardware
    // Notificar al kernel
}
```

## Integración con ReactOS

### Proceso de Adaptación

1. **Selección**: Identificar driver de ReactOS a adaptar
2. **Extracción**: Extraer código fuente del driver
3. **Análisis**: Entender dependencias y requisitos
4. **Adaptación**: Modificar para nuestro kernel
5. **Prueba**: Verificar funcionamiento
6. **Documentación**: Documentar cambios realizados

### Consideraciones

- Mantener créditos originales de ReactOS
- Documentar modificaciones realizadas
- Respetar la licencia GPL
- Mantener compatibilidad cuando sea posible

## Estado Actual

**Pendiente de implementación**

- [ ] Framework básico de drivers
- [ ] Driver de teclado PS/2
- [ ] Driver de video VGA
- [ ] Driver de disco IDE
- [ ] Sistema de archivos FAT

## Referencias

- [ReactOS Drivers](https://github.com/reactos/reactos/tree/master/drivers)
- [OSDev - Drivers](https://wiki.osdev.org/Category:Drivers)
- [Linux Driver Development](https://www.kernel.org/doc/html/latest/driver-api/)

## Notas

- Los drivers deben ser modulares y fáciles de reemplazar
- Priorizar estabilidad sobre características avanzadas
- Implementar logging extensivo para depuración
- Considerar soporte para hot-plug en el futuro
