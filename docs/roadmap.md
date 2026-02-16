# Roadmap de Desarrollo

Este documento describe el plan de desarrollo del sistema operativo personalizado en fases progresivas.

## Visión General

El desarrollo sigue un enfoque incremental, construyendo funcionalidad básica primero y agregando características más avanzadas gradualmente.

**Duración Estimada Total**: 12-18 meses (dependiendo de recursos)

---

## Fase 0: Estructura Inicial ✅ (COMPLETADO)

**Duración**: 1 semana

### Objetivos
- [x] Estructura de directorios del proyecto
- [x] Documentación inicial
- [x] Sistema de compilación básico (Makefile)
- [x] Configuración de repositorio Git
- [x] Licencia y créditos

### Entregables
- [x] README.md completo
- [x] Estructura /boot, /kernel, /drivers, /lib, /docs, /tools
- [x] Makefile funcional
- [x] Documentación de cada módulo
- [x] Arquitectura documentada
- [x] Roadmap (este documento)

---

## Fase 1: Bootloader Funcional

**Duración Estimada**: 4-6 semanas

**Estado**: 🔴 Pendiente

### Objetivos
Crear un bootloader básico que pueda cargar el kernel en memoria.

### Tareas

#### 1.1 Bootloader Básico (2 semanas)
- [ ] Implementar sector de arranque (Stage 1)
  - [ ] Código Assembly para MBR
  - [ ] Cargar Stage 2 desde disco
  - [ ] Pasar control a Stage 2
  
- [ ] Implementar Stage 2 básico
  - [ ] Inicializar modo protegido
  - [ ] Cargar kernel desde disco
  - [ ] Saltar a punto de entrada del kernel

#### 1.2 Detección de Hardware (1 semana)
- [ ] Detectar memoria disponible (E820 BIOS call)
- [ ] Detectar dispositivos de arranque
- [ ] Crear mapa de memoria para el kernel

#### 1.3 Integración FreeLoader (2-3 semanas)
- [ ] Extraer código de FreeLoader desde ReactOS
- [ ] Adaptar para nuestro kernel
- [ ] Implementar carga de drivers iniciales
- [ ] Configuración de arranque (freeldr.ini)

### Entregables
- [ ] Bootloader que carga el kernel
- [ ] Documentación del proceso de arranque
- [ ] Scripts de creación de imágenes booteables

### Pruebas
- [ ] Boot en QEMU
- [ ] Boot en VirtualBox
- [ ] Boot en hardware real (opcional)

---

## Fase 2: Kernel Básico

**Duración Estimada**: 6-8 semanas

**Estado**: 🟡 En Desarrollo Inicial

### Objetivos
Implementar las funcionalidades fundamentales del kernel.

### Tareas

#### 2.1 Tablas de Descriptores (1 semana)
- [ ] Implementar GDT (Global Descriptor Table)
  - [ ] Segmentos de código y datos
  - [ ] Configuración de privilegios
  
- [ ] Implementar IDT (Interrupt Descriptor Table)
  - [ ] Vectores de interrupción
  - [ ] Handlers de excepciones

#### 2.2 Gestión de Interrupciones (2 semanas)
- [ ] Handlers de excepciones de CPU
  - [ ] Division by zero
  - [ ] Page fault
  - [ ] General protection fault
  - [ ] Otras excepciones
  
- [ ] Handlers de IRQ
  - [ ] PIC (Programmable Interrupt Controller)
  - [ ] Timer interrupt (IRQ 0)
  - [ ] Keyboard interrupt (IRQ 1)

#### 2.3 Gestión de Memoria Básica (2-3 semanas)
- [ ] Physical Memory Manager (PMM)
  - [ ] Bitmap de páginas físicas
  - [ ] Asignación/liberación de páginas
  
- [ ] Virtual Memory Manager (VMM)
  - [ ] Page Directory y Page Tables
  - [ ] Mapeo de memoria
  - [ ] Habilitar paginación
  
- [ ] Heap del Kernel
  - [ ] kmalloc/kfree básico
  - [ ] Gestión de bloques

#### 2.4 Timer y Clock (1 semana)
- [ ] Configurar PIT (Programmable Interval Timer)
- [ ] Contador de ticks del sistema
- [ ] Funciones de delay

#### 2.5 Entrada/Salida Básica (1 semana)
- [ ] Mejorar driver VGA
  - [ ] Scroll de pantalla
  - [ ] Colores y atributos
  
- [ ] Puerto serial para debug
  - [ ] Salida a COM1
  - [ ] Logging del kernel

### Entregables
- [ ] Kernel con interrupciones funcionando
- [ ] Sistema de memoria virtual
- [ ] Timer funcionando
- [ ] I/O básico funcional

### Pruebas
- [ ] Test de interrupciones
- [ ] Test de asignación de memoria
- [ ] Test del timer

---

## Fase 3: HAL (Hardware Abstraction Layer)

**Duración Estimada**: 4-6 semanas

**Estado**: 🔴 Pendiente

### Objetivos
Integrar y adaptar el HAL de ReactOS.

### Tareas

#### 3.1 Extracción de HAL (1 semana)
- [ ] Extraer código HAL de ReactOS
- [ ] Identificar dependencias
- [ ] Análisis de código

#### 3.2 Adaptación de HAL (2-3 semanas)
- [ ] Adaptar inicialización de HAL
- [ ] Adaptar gestión de interrupciones
- [ ] Adaptar acceso a I/O
- [ ] Adaptar timers y RTC

#### 3.3 Integración con Kernel (1-2 semanas)
- [ ] Modificar kernel para usar HAL
- [ ] Actualizar gestión de interrupciones
- [ ] Actualizar acceso a hardware
- [ ] Testing de integración

### Entregables
- [ ] HAL funcional integrado
- [ ] Documentación de adaptación
- [ ] Tests de HAL

---

## Fase 4: Drivers Básicos

**Duración Estimada**: 6-8 semanas

**Estado**: 🔴 Pendiente

### Objetivos
Implementar drivers esenciales para funcionalidad básica.

### Tareas

#### 4.1 Framework de Drivers (1 semana)
- [ ] Sistema de registro de drivers
- [ ] I/O Manager básico
- [ ] Interfaz de drivers

#### 4.2 Driver de Teclado (1 semana)
- [ ] Driver PS/2 keyboard
- [ ] Buffer de teclado
- [ ] Mapeo de teclas
- [ ] Soporte para diferentes layouts

#### 4.3 Driver de Disco IDE (2-3 semanas)
- [ ] Detección de discos IDE
- [ ] Lectura de sectores
- [ ] Escritura de sectores
- [ ] DMA transfer (opcional)

#### 4.4 Driver de Sistema de Archivos FAT (2-3 semanas)
- [ ] Lectura de FAT16/FAT32
- [ ] Navegación de directorios
- [ ] Lectura de archivos
- [ ] Escritura de archivos

#### 4.5 Virtual File System (VFS) (1 semana)
- [ ] Interfaz abstracta de filesystem
- [ ] Montaje de filesystems
- [ ] Path resolution

### Entregables
- [ ] Driver de teclado funcional
- [ ] Driver de disco funcional
- [ ] Sistema de archivos FAT operativo
- [ ] VFS básico

### Pruebas
- [ ] Lectura de archivos desde disco
- [ ] Escritura de archivos a disco
- [ ] Navegación de directorios

---

## Fase 5: Gestión de Procesos

**Duración Estimada**: 6-8 semanas

**Estado**: 🔴 Pendiente

### Objetivos
Implementar multitasking y gestión de procesos.

### Tareas

#### 5.1 Estructuras de Procesos (1 semana)
- [ ] Process Control Block (PCB)
- [ ] Thread Control Block (TCB)
- [ ] Listas de procesos

#### 5.2 Scheduler (2 semanas)
- [ ] Scheduler round-robin
- [ ] Colas de procesos
- [ ] Cambio de contexto
- [ ] Process/thread states

#### 5.3 User Mode (2 semanas)
- [ ] Transición a Ring 3
- [ ] System calls
- [ ] User space memory layout

#### 5.4 Process Creation (2 semanas)
- [ ] Cargar ejecutables (ELF o PE simple)
- [ ] Crear proceso inicial
- [ ] fork/exec (versión simple)

#### 5.5 Sincronización (1 semana)
- [ ] Mutexes
- [ ] Semáforos
- [ ] Events

### Entregables
- [ ] Multitasking funcionando
- [ ] System calls implementadas
- [ ] Procesos de usuario ejecutándose

### Pruebas
- [ ] Múltiples procesos corriendo
- [ ] Cambio de contexto
- [ ] System calls

---

## Fase 6: Shell Simple

**Duración Estimada**: 3-4 semanas

**Estado**: 🔴 Pendiente

### Objetivos
Crear una shell de línea de comandos básica.

### Tareas

#### 6.1 Shell Básica (2 semanas)
- [ ] Prompt de comandos
- [ ] Parser de comandos
- [ ] Comandos internos básicos
  - [ ] ls (listar archivos)
  - [ ] cd (cambiar directorio)
  - [ ] cat (mostrar archivo)
  - [ ] echo
  - [ ] clear

#### 6.2 Ejecutar Programas (1 semana)
- [ ] Cargar y ejecutar programas externos
- [ ] Gestión de argumentos
- [ ] Variables de entorno básicas

#### 6.3 I/O Redirection (opcional, 1 semana)
- [ ] Redirección de entrada/salida
- [ ] Pipes básicos

### Entregables
- [ ] Shell funcional
- [ ] Comandos básicos implementados
- [ ] Capacidad de ejecutar programas

---

## Fase 7: Características Avanzadas

**Duración Estimada**: Variable

**Estado**: 🔴 Futuro

### Áreas de Desarrollo

#### 7.1 Networking
- [ ] Stack TCP/IP
- [ ] Drivers de red
- [ ] Sockets

#### 7.2 USB Support
- [ ] USB Host Controller drivers
- [ ] USB device support

#### 7.3 Graphics
- [ ] Framebuffer
- [ ] GUI básica
- [ ] Window manager

#### 7.4 Advanced Filesystems
- [ ] NTFS completo
- [ ] ext2/ext3/ext4
- [ ] Journaling

#### 7.5 SMP (Symmetric Multi-Processing)
- [ ] Soporte multi-core
- [ ] Load balancing
- [ ] IPI (Inter-Processor Interrupts)

---

## Métricas de Éxito

### Fase 1
- ✅ El sistema arranca desde disco/USB/CD
- ✅ El kernel se carga correctamente

### Fase 2
- ✅ Interrupciones funcionan correctamente
- ✅ Memoria virtual operativa
- ✅ Timer funcionando

### Fase 3
- ✅ HAL integrado sin errores
- ✅ Abstracción de hardware funcional

### Fase 4
- ✅ Se puede leer/escribir archivos desde disco
- ✅ Teclado funcional

### Fase 5
- ✅ Múltiples procesos ejecutándose
- ✅ System calls funcionando

### Fase 6
- ✅ Shell funcional con comandos básicos
- ✅ Se pueden ejecutar programas

---

## Notas Importantes

### Prioridades
1. **Estabilidad** sobre características
2. **Simplicidad** sobre complejidad
3. **Documentación** en cada fase
4. **Testing** continuo

### Flexibilidad
Este roadmap es flexible y puede ajustarse según:
- Dificultades técnicas encontradas
- Disponibilidad de recursos
- Feedback de la comunidad
- Nuevas prioridades

### Dependencias de ReactOS
En cada fase, documentar:
- Qué código viene de ReactOS
- Qué modificaciones se hicieron
- Por qué se hicieron esas modificaciones

---

## Versiones Planificadas

### v0.1 - "Bootstrap" (Actual)
- Estructura del proyecto
- Documentación inicial
- Kernel minimalista

### v0.2 - "Bootable"
- Bootloader funcional
- Kernel mejorado
- HAL básico

### v0.3 - "Interactive"
- Drivers básicos
- Sistema de archivos
- Entrada de teclado

### v0.4 - "Multitasking"
- Gestión de procesos
- Scheduler
- System calls

### v0.5 - "Usable"
- Shell funcional
- Comandos básicos
- Utilidades básicas

### v1.0 - "Stable"
- Sistema completo y estable
- Documentación completa
- Conjunto de utilidades
- Tests exhaustivos

---

## Contribuciones

Buscamos contribuciones en todas las áreas:
- Código del kernel
- Drivers
- Documentación
- Testing
- Herramientas de desarrollo

Para contribuir, ver [docs/README.md](README.md) sección de contribución.

---

## Referencias

- [ReactOS Roadmap](https://reactos.org/roadmap/)
- [Linux Kernel Development](https://www.kernel.org/)
- [OSDev Wiki](https://wiki.osdev.org/)

---

**Última actualización**: Fecha de creación del proyecto
**Siguiente revisión**: Al completar Fase 1
