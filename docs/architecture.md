# Arquitectura del Sistema Operativo

## Visión General

Este documento describe la arquitectura general del sistema operativo personalizado basado en componentes de ReactOS.

## Diseño General

### Modelo de Arquitectura

El sistema operativo sigue un modelo **híbrido** similar a Windows NT y ReactOS:

```
┌─────────────────────────────────────────────────────┐
│           Aplicaciones de Usuario (Ring 3)           │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐          │
│  │  Shell   │  │Utilidades│  │Programas │          │
│  └──────────┘  └──────────┘  └──────────┘          │
└─────────────────────────────────────────────────────┘
         │              │              │
         ▼              ▼              ▼
┌─────────────────────────────────────────────────────┐
│              System Call Interface                   │
└─────────────────────────────────────────────────────┘
         │              │              │
         ▼              ▼              ▼
┌─────────────────────────────────────────────────────┐
│            Kernel Mode (Ring 0)                      │
│  ┌────────────────────────────────────────────┐     │
│  │         Executive Services                  │     │
│  │  ┌─────────┐  ┌──────────┐  ┌──────────┐  │     │
│  │  │I/O Mgr  │  │Memory Mgr│  │Process   │  │     │
│  │  │         │  │          │  │Manager   │  │     │
│  │  └─────────┘  └──────────┘  └──────────┘  │     │
│  └────────────────────────────────────────────┘     │
│  ┌────────────────────────────────────────────┐     │
│  │              Kernel                         │     │
│  │  ┌──────────┐  ┌──────────┐  ┌─────────┐  │     │
│  │  │Scheduler │  │Interrupt │  │Memory   │  │     │
│  │  │          │  │Handler   │  │Manager  │  │     │
│  │  └──────────┘  └──────────┘  └─────────┘  │     │
│  └────────────────────────────────────────────┘     │
│  ┌────────────────────────────────────────────┐     │
│  │    Hardware Abstraction Layer (HAL)        │     │
│  └────────────────────────────────────────────┘     │
└─────────────────────────────────────────────────────┘
         │              │              │
         ▼              ▼              ▼
┌─────────────────────────────────────────────────────┐
│             Hardware (CPU, Memoria, I/O)            │
└─────────────────────────────────────────────────────┘
```

## Componentes Principales

### 1. Bootloader (FreeLoader)

**Propósito**: Cargar el kernel y transferir el control del sistema.

**Responsabilidades**:
- Inicializar hardware básico
- Detectar memoria disponible
- Cargar el kernel en memoria
- Cargar drivers iniciales
- Configurar estructuras de datos iniciales
- Transferir control al kernel

**Código Fuente**: Adaptado de ReactOS FreeLoader

### 2. Hardware Abstraction Layer (HAL)

**Propósito**: Abstraer las diferencias de hardware específicas.

**Responsabilidades**:
- Acceso a puertos I/O
- Gestión de interrupciones
- Gestión de DMA
- Acceso a timers y RTC
- Gestión de ACPI/APM
- Soporte multi-procesador (SMP)

**Interfaces Principales**:
```c
// Gestión de interrupciones
VOID HalInitializeInterrupts(VOID);
VOID HalEnableInterrupt(UCHAR Vector);
VOID HalDisableInterrupt(UCHAR Vector);

// Acceso a puertos
UCHAR HalReadPort8(USHORT Port);
VOID HalWritePort8(USHORT Port, UCHAR Value);

// Timers
ULONGLONG HalGetTickCount(VOID);
VOID HalInitializeTimer(VOID);
```

**Código Fuente**: Adaptado de ReactOS HAL

### 3. Kernel

**Propósito**: Núcleo del sistema operativo, gestiona recursos fundamentales.

#### 3.1 Gestor de Memoria

**Responsabilidades**:
- Gestión de memoria física (PMM)
- Gestión de memoria virtual (VMM)
- Paginación
- Heap del kernel (kmalloc/kfree)
- Memory mapping

**Estructuras Clave**:
```c
typedef struct _MEMORY_DESCRIPTOR {
    PVOID BaseAddress;
    SIZE_T Size;
    ULONG Type;
    ULONG Flags;
} MEMORY_DESCRIPTOR, *PMEMORY_DESCRIPTOR;

typedef struct _PAGE_TABLE_ENTRY {
    ULONG Present : 1;
    ULONG ReadWrite : 1;
    ULONG User : 1;
    ULONG WriteThrough : 1;
    ULONG CacheDisable : 1;
    ULONG Accessed : 1;
    ULONG Dirty : 1;
    ULONG PageSize : 1;
    ULONG Global : 1;
    ULONG Available : 3;
    ULONG PageFrameNumber : 20;
} PAGE_TABLE_ENTRY, *PPAGE_TABLE_ENTRY;
```

#### 3.2 Gestor de Procesos

**Responsabilidades**:
- Creación y terminación de procesos
- Scheduling (planificación)
- Cambio de contexto
- Gestión de threads
- Sincronización (mutexes, semáforos)

**Estructuras Clave**:
```c
typedef struct _PROCESS {
    ULONG ProcessId;
    CHAR Name[256];
    PVOID PageDirectory;
    LIST_ENTRY ThreadList;
    PROCESS_STATE State;
    ULONG Priority;
} PROCESS, *PPROCESS;

typedef struct _THREAD {
    ULONG ThreadId;
    PPROCESS Process;
    PVOID StackPointer;
    THREAD_STATE State;
    ULONG Priority;
    CONTEXT Context;
} THREAD, *PTHREAD;
```

#### 3.3 Gestor de Interrupciones

**Responsabilidades**:
- Configurar IDT (Interrupt Descriptor Table)
- Manejar excepciones de CPU
- Manejar IRQs de hardware
- Dispatch de interrupciones

**Vectores de Interrupción**:
```
0-31:   Excepciones de CPU (división por cero, page fault, etc.)
32-47:  IRQs de hardware (timer, teclado, disco, etc.)
48+:    Interrupciones de software (syscalls)
```

### 4. Executive Services

**Propósito**: Servicios de alto nivel del sistema.

#### 4.1 I/O Manager

**Responsabilidades**:
- Gestión de drivers
- Sistema de archivos virtual (VFS)
- Buffering y caching
- Asynchronous I/O

#### 4.2 Object Manager

**Responsabilidades**:
- Gestión de objetos del sistema
- Namespace del sistema
- Referencias y handles
- Seguridad de objetos

#### 4.3 Security Reference Monitor

**Responsabilidades**:
- Control de acceso
- Tokens de seguridad
- Auditoría
- Privilegios

### 5. Drivers

**Propósito**: Interactuar con hardware específico.

**Tipos de Drivers**:
- **Drivers de Disco**: IDE, SATA, AHCI
- **Drivers de Entrada**: Teclado, Mouse
- **Drivers de Video**: VGA, VESA
- **Drivers de Red**: Ethernet, WiFi
- **Drivers de Sistema de Archivos**: FAT, NTFS

**Modelo de Driver**:
```c
typedef struct _DRIVER_OBJECT {
    PDEVICE_OBJECT DeviceObject;
    PDRIVER_DISPATCH MajorFunction[IRP_MJ_MAXIMUM_FUNCTION];
    PDRIVER_INITIALIZE DriverInit;
    PDRIVER_UNLOAD DriverUnload;
} DRIVER_OBJECT, *PDRIVER_OBJECT;
```

### 6. Runtime Libraries

**Propósito**: Funciones comunes compartidas.

**Librerías**:
- **RTL (Runtime Library)**: Strings, memory, listas
- **CRT (C Runtime)**: Funciones estándar C
- **Kernel Library**: Spinlocks, mutexes, debug

## Flujo de Arranque

### Secuencia Completa de Boot

```
1. BIOS/UEFI Power-On
   │
   ├─▶ POST (Power-On Self Test)
   │
   ├─▶ Inicialización de hardware básico
   │
   └─▶ Buscar dispositivo booteable
       │
       ▼
2. Bootloader Stage 1
   │
   ├─▶ Cargar desde MBR/GPT
   │
   ├─▶ Inicializar sistema de archivos
   │
   └─▶ Cargar Stage 2
       │
       ▼
3. Bootloader Stage 2 (FreeLoader)
   │
   ├─▶ Mostrar menú de boot
   │
   ├─▶ Leer configuración
   │
   ├─▶ Detectar memoria (E820)
   │
   ├─▶ Habilitar A20 gate
   │
   ├─▶ Configurar modo protegido
   │
   ├─▶ Cargar kernel en memoria
   │
   ├─▶ Cargar drivers iniciales
   │
   └─▶ Saltar a kernel_main
       │
       ▼
4. Kernel Initialization
   │
   ├─▶ kernel_main()
   │   │
   │   ├─▶ Inicializar terminal/video
   │   │
   │   ├─▶ Inicializar GDT
   │   │
   │   ├─▶ Inicializar IDT
   │   │
   │   ├─▶ Inicializar HAL
   │   │   ├─▶ Detectar CPU
   │   │   ├─▶ Configurar APIC/PIC
   │   │   └─▶ Inicializar timers
   │   │
   │   ├─▶ Inicializar Memory Manager
   │   │   ├─▶ Physical Memory Manager
   │   │   ├─▶ Virtual Memory Manager
   │   │   └─▶ Habilitar paginación
   │   │
   │   ├─▶ Inicializar Process Manager
   │   │   ├─▶ Crear proceso idle
   │   │   └─▶ Inicializar scheduler
   │   │
   │   ├─▶ Inicializar I/O Manager
   │   │   └─▶ Cargar drivers
   │   │
   │   ├─▶ Montar sistema de archivos root
   │   │
   │   └─▶ Crear proceso init/shell
   │       │
   │       ▼
5. Sistema Operativo Running
   │
   ├─▶ Scheduler ejecutándose
   │
   ├─▶ Interrupciones habilitadas
   │
   └─▶ Sistema listo para aplicaciones
```

## Diseño Modular

### Principios de Diseño

1. **Separación de Concerns**: Cada módulo tiene responsabilidades claras
2. **Interfaces Definidas**: APIs bien documentadas entre módulos
3. **Mínimas Dependencias**: Reducir acoplamiento entre módulos
4. **Extensibilidad**: Fácil agregar nuevas características
5. **Portabilidad**: Código específico de arquitectura aislado en HAL

### Dependencias entre Módulos

```
Kernel
  ├─▶ HAL (Hardware Abstraction Layer)
  ├─▶ RTL (Runtime Library)
  └─▶ Libc (C Runtime - subset)

Executive
  ├─▶ Kernel
  ├─▶ HAL
  └─▶ RTL

Drivers
  ├─▶ Kernel
  ├─▶ HAL
  └─▶ I/O Manager

Applications
  ├─▶ System Calls
  └─▶ User-mode libraries
```

## Gestión de Memoria

### Layout de Memoria Virtual (x86 32-bit)

```
0x00000000 - 0x003FFFFF  4MB:   Kernel code y data
0x00400000 - 0x7FFFFFFF  ~2GB:  User space
0x80000000 - 0xBFFFFFFF  1GB:   Kernel heap y drivers
0xC0000000 - 0xFFFFFFFF  1GB:   Memoria mapeada y reservada
```

### Paginación

- **Page Size**: 4KB (4096 bytes)
- **Page Directory**: 1024 entradas
- **Page Table**: 1024 entradas por tabla
- **Total Virtual Memory**: 4GB (2^32)

## Sincronización

### Mecanismos de Sincronización

1. **Spinlocks**: Para protección de datos en kernel
2. **Mutexes**: Para sincronización entre threads
3. **Semáforos**: Para control de recursos limitados
4. **Events**: Para señalización entre procesos

## System Calls

### Interfaz de System Call

```c
// Ejemplo de system call
NTSTATUS NtCreateFile(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER AllocationSize OPTIONAL,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength
);
```

### Proceso de System Call

```
1. Application llama función de librería (wrapper)
2. Wrapper prepara parámetros en registros/stack
3. Ejecuta instrucción INT 0x80 o SYSCALL
4. CPU cambia a Ring 0 (kernel mode)
5. Kernel dispatcher identifica system call
6. Kernel ejecuta función correspondiente
7. Resultado se retorna a user mode
8. Application continúa ejecución
```

## Seguridad

### Niveles de Privilegio (Rings)

- **Ring 0**: Kernel mode - Acceso completo al hardware
- **Ring 1-2**: No utilizados en x86 (reservados)
- **Ring 3**: User mode - Acceso restringido

### Protección de Memoria

- Páginas marcadas como kernel/user
- Páginas marcadas como read-only/read-write
- NX bit para prevenir ejecución de datos

## Conclusión

Esta arquitectura proporciona:
- **Modularidad**: Componentes independientes y reemplazables
- **Escalabilidad**: Fácil agregar nuevas características
- **Portabilidad**: HAL permite soportar diferentes arquitecturas
- **Mantenibilidad**: Código organizado y bien documentado

El uso de componentes de ReactOS acelera el desarrollo mientras mantiene compatibilidad con estándares bien establecidos.
