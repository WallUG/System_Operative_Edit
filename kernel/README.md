# Kernel - Núcleo del Sistema Operativo

## Descripción

Este directorio contiene el kernel (núcleo) del sistema operativo, el componente central que gestiona los recursos del sistema.

## Arquitectura del Kernel

### Diseño Modular
El kernel está diseñado con una arquitectura modular que incluye:

- **Gestión de Memoria**: Asignación y liberación de memoria
- **Gestión de Procesos**: Creación, planificación y terminación de procesos
- **Gestión de Interrupciones**: Manejo de interrupciones de hardware y software
- **Sistema de Archivos**: Interfaz para acceso a archivos
- **Gestión de Dispositivos**: Interfaz con drivers de hardware

### Modelo de Kernel
- Tipo: Kernel híbrido (inspirado en Windows NT/ReactOS)
- Modo de ejecución: Protección de anillos (Ring 0 para kernel, Ring 3 para aplicaciones)
- Arquitectura objetivo: x86 (32-bit inicialmente)

## Punto de Entrada

### kernel_main()
La función `kernel_main()` en `main.c` es el punto de entrada principal del kernel.

**Responsabilidades:**
1. Inicializar la salida de video/terminal
2. Mostrar información de arranque
3. Inicializar el HAL (Hardware Abstraction Layer)
4. Cargar y inicializar drivers
5. Configurar la gestión de memoria
6. Inicializar el scheduler de procesos
7. Cargar el proceso inicial (shell o init)

### Flujo de Ejecución

```
Bootloader
    ↓
_start() / kernel_main()
    ↓
Inicializar video/terminal
    ↓
Inicializar HAL
    ↓
Configurar GDT/IDT
    ↓
Inicializar memoria
    ↓
Cargar drivers
    ↓
Inicializar scheduler
    ↓
Ejecutar proceso inicial
    ↓
Loop de scheduler
```

## Estructura de Archivos

```
/kernel
├── main.c              # Punto de entrada y lógica principal
├── linker.ld           # Script del linker
├── memory/             # Gestión de memoria (futuro)
│   ├── pmm.c          # Physical Memory Manager
│   ├── vmm.c          # Virtual Memory Manager
│   └── heap.c         # Gestión de heap
├── process/            # Gestión de procesos (futuro)
│   ├── scheduler.c    # Planificador de procesos
│   ├── thread.c       # Gestión de hilos
│   └── context.c      # Cambio de contexto
├── interrupt/          # Gestión de interrupciones (futuro)
│   ├── idt.c          # Interrupt Descriptor Table
│   ├── irq.c          # IRQ handlers
│   └── exceptions.c   # Manejo de excepciones
├── io/                 # Entrada/Salida (futuro)
│   ├── vga.c          # Driver VGA básico
│   └── serial.c       # Puerto serial para debug
└── include/            # Headers del kernel (futuro)
    ├── kernel.h       # Definiciones principales
    ├── types.h        # Tipos de datos
    └── memory.h       # Definiciones de memoria
```

## Componentes de ReactOS

### HAL (Hardware Abstraction Layer)
Proporcionará abstracción del hardware específico:
- Gestión de interrupciones
- Acceso a hardware
- Temporizadores y RTC
- DMA (Direct Memory Access)

### Executive
Servicios del kernel de alto nivel:
- Gestión de objetos
- I/O Manager
- Process Manager
- Memory Manager

## Planes de Desarrollo

### Fase 1: Kernel Básico (Actual)
- [x] Punto de entrada funcional
- [x] Salida a pantalla (VGA)
- [ ] Gestión de interrupciones básica
- [ ] GDT (Global Descriptor Table)
- [ ] IDT (Interrupt Descriptor Table)

### Fase 2: Memoria
- [ ] Physical Memory Manager
- [ ] Virtual Memory Manager
- [ ] Heap allocation (kmalloc/kfree)
- [ ] Paginación

### Fase 3: Procesos
- [ ] Estructura de procesos
- [ ] Scheduler round-robin básico
- [ ] Cambio de contexto
- [ ] Syscalls básicas

### Fase 4: Integración HAL
- [ ] Adaptar HAL de ReactOS
- [ ] Abstracción de hardware
- [ ] Soporte multi-arquitectura

## Compilación

El kernel se compila con:
```bash
make kernel
```

Esto genera `build/kernel.bin`, el binario del kernel que será cargado por el bootloader.

## Depuración

### Con QEMU y GDB
```bash
# Terminal 1: Ejecutar QEMU con servidor GDB
qemu-system-i386 -kernel build/kernel.bin -s -S

# Terminal 2: Conectar GDB
gdb build/kernel.bin
(gdb) target remote localhost:1234
(gdb) break kernel_main
(gdb) continue
```

### Puerto Serial
El kernel puede configurarse para enviar mensajes de debug a un puerto serial:
```bash
qemu-system-i386 -kernel build/kernel.bin -serial stdio
```

## Convenciones de Código

- **Estilo**: K&R style
- **Indentación**: 4 espacios
- **Nombres**: snake_case para funciones, UPPER_CASE para macros
- **Comentarios**: Explicar el "por qué", no el "qué"

## Referencias

- [ReactOS Kernel](https://github.com/reactos/reactos/tree/master/ntoskrnl)
- [OSDev Wiki](https://wiki.osdev.org/)
- [Intel x86 Manual](https://software.intel.com/content/www/us/en/develop/articles/intel-sdm.html)

## Estado Actual

**En desarrollo inicial**

El kernel actual es minimalista y solo puede:
- Arrancar
- Mostrar texto en pantalla
- Entrar en loop infinito (halt)

Los siguientes pasos son implementar interrupciones, memoria y procesos básicos.
