# Librerías - Runtime y Soporte del Sistema

## Descripción

Este directorio contiene las librerías compartidas y de runtime que proporcionan funcionalidad común al kernel y a las aplicaciones de usuario.

## Librerías Planificadas

### 1. Runtime Library (RTL)

La Runtime Library proporciona funciones básicas utilizadas por todo el sistema.

**Fuente**: Adaptada de ReactOS RTL

**Componentes principales**:
- Manipulación de strings
- Manejo de memoria
- Estructuras de datos (listas, árboles, hash tables)
- Funciones matemáticas básicas
- Utilidades de tiempo y fecha

**Archivos**:
```
/lib/rtl/
├── string.c        # strlen, strcpy, strcmp, etc.
├── memory.c        # memcpy, memset, memmove
├── list.c          # Listas enlazadas
├── bitmap.c        # Operaciones de bitmap
├── unicode.c       # Soporte Unicode
└── time.c          # Funciones de tiempo
```

### 2. Kernel Support Library

Funciones específicas para uso del kernel.

**Componentes**:
- Locks y sincronización
- Manejo de estructuras del kernel
- Pool allocation
- Funciones de debug

**Archivos**:
```
/lib/kernel/
├── spinlock.c      # Spinlocks
├── mutex.c         # Mutexes
├── semaphore.c     # Semáforos
└── debug.c         # Funciones de debug
```

### 3. C Standard Library (libc) - Subconjunto

Implementación parcial de la librería estándar C para el kernel.

**Funciones incluidas**:
- stdio: printf, sprintf (versiones kernel)
- string: strlen, strcmp, strcpy, etc.
- stdlib: atoi, itoa, malloc, free
- ctype: isdigit, isalpha, etc.

**Nota**: No se incluyen funciones que requieren sistema de archivos o networking hasta que estén disponibles.

**Archivos**:
```
/lib/libc/
├── stdio/
│   ├── printf.c
│   └── sprintf.c
├── string/
│   ├── strlen.c
│   ├── strcmp.c
│   └── strcpy.c
├── stdlib/
│   ├── atoi.c
│   └── itoa.c
└── ctype/
    └── ctype.c
```

### 4. HAL Library

Funciones de abstracción de hardware.

**Fuente**: Adaptada de ReactOS HAL

**Componentes**:
- Acceso a puertos I/O
- Gestión de interrupciones
- Timers y contadores
- DMA
- Configuración de CPU

**Archivos**:
```
/lib/hal/
├── io.c            # Port I/O functions
├── interrupt.c     # Interrupt management
├── timer.c         # Timer functions
├── dma.c           # DMA operations
└── cpu.c           # CPU configuration
```

### 5. Matemáticas

Funciones matemáticas básicas.

**Componentes**:
- Operaciones aritméticas
- Operaciones bit a bit
- Funciones trigonométricas (futuro)

**Archivos**:
```
/lib/math/
├── basic.c         # +, -, *, /
├── bitops.c        # Operaciones de bits
└── trig.c          # Trigonometría (futuro)
```

## Librerías de ReactOS a Reutilizar

### RTL (Runtime Library)
- **Ubicación en ReactOS**: `sdk/lib/rtl/`
- **Descripción**: Funciones fundamentales de runtime
- **Prioridad**: Alta

Funciones importantes:
```c
// Strings
NTSTATUS RtlInitUnicodeString();
NTSTATUS RtlCompareUnicodeString();

// Memory
VOID RtlZeroMemory();
VOID RtlCopyMemory();

// Lists
VOID InitializeListHead();
VOID InsertHeadList();
VOID RemoveEntryList();
```

### CRT (C Runtime)
- **Ubicación en ReactOS**: `sdk/lib/crt/`
- **Descripción**: Implementación de funciones C estándar
- **Prioridad**: Alta

### NDK (Native Development Kit)
- **Ubicación en ReactOS**: `sdk/include/ndk/`
- **Descripción**: Headers para desarrollo nativo
- **Prioridad**: Media

## Estructura de Archivos Completa

```
/lib
├── rtl/                # Runtime Library
│   ├── string.c
│   ├── memory.c
│   ├── list.c
│   ├── bitmap.c
│   ├── unicode.c
│   └── time.c
├── kernel/             # Kernel Support Library
│   ├── spinlock.c
│   ├── mutex.c
│   ├── semaphore.c
│   └── debug.c
├── libc/               # C Standard Library (subset)
│   ├── stdio/
│   ├── string/
│   ├── stdlib/
│   └── ctype/
├── hal/                # Hardware Abstraction Layer Library
│   ├── io.c
│   ├── interrupt.c
│   ├── timer.c
│   ├── dma.c
│   └── cpu.c
├── math/               # Mathematical functions
│   ├── basic.c
│   ├── bitops.c
│   └── trig.c
├── include/            # Header files
│   ├── rtl.h
│   ├── kernel.h
│   ├── hal.h
│   └── types.h
└── README.md
```

## Guía de Uso

### Incluir Librerías en el Kernel

```c
#include <lib/rtl.h>
#include <lib/kernel.h>

void kernel_function() {
    // Usar funciones RTL
    char *str = "Hello";
    size_t len = rtl_strlen(str);
    
    // Usar funciones de kernel
    spinlock_acquire(&my_lock);
    // Critical section
    spinlock_release(&my_lock);
}
```

### Compilar Librerías

Las librerías se compilan como parte del proceso de compilación del kernel:

```bash
make libs      # Compilar todas las librerías
make rtl       # Compilar solo RTL
make libc      # Compilar solo libc
```

## Convenciones

### Nombres de Funciones

- **RTL**: Prefijo `rtl_` o `Rtl`
  - Ejemplo: `rtl_strlen()`, `RtlZeroMemory()`

- **Kernel**: Prefijo `k_` o `kernel_`
  - Ejemplo: `k_malloc()`, `kernel_init()`

- **HAL**: Prefijo `hal_` o `Hal`
  - Ejemplo: `hal_read_port()`, `HalInitializeInterrupts()`

### Estilo de Código

- K&R style para funciones C nuevas
- Mantener estilo original de ReactOS donde se adapte código
- Comentarios en español para código nuevo
- Mantener comentarios originales en inglés donde aplique

## Planes de Desarrollo

### Fase 1: Librerías Básicas
- [ ] Funciones de string básicas
- [ ] Funciones de memoria básicas
- [ ] printf para kernel (kprintf)

### Fase 2: RTL Completa
- [ ] Listas enlazadas
- [ ] Bitmaps
- [ ] Soporte Unicode básico

### Fase 3: Kernel Support
- [ ] Spinlocks
- [ ] Mutexes
- [ ] Semáforos

### Fase 4: HAL Library
- [ ] Port I/O
- [ ] Gestión de interrupciones
- [ ] Timers

## Referencias

- [ReactOS RTL](https://github.com/reactos/reactos/tree/master/sdk/lib/rtl)
- [ReactOS CRT](https://github.com/reactos/reactos/tree/master/sdk/lib/crt)
- [Windows NT Native API](https://undocumented.ntinternals.net/)

## Estado Actual

**Pendiente de implementación**

- [ ] Estructura básica de librerías
- [ ] Funciones de string
- [ ] Funciones de memoria
- [ ] kprintf para debug

## Notas

- Las librerías deben ser thread-safe donde sea necesario
- Priorizar funciones más usadas primero
- Mantener compatibilidad con ReactOS cuando sea posible
- Documentar bien las diferencias con implementaciones estándar
