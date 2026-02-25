# Documentación del Sistema Operativo Personalizado

## Índice

Este directorio contiene toda la documentación del proyecto.

### Documentos Principales

1. **[Arquitectura del Sistema](architecture.md)**
   - Diseño general del sistema
   - Componentes principales
   - Flujo de arranque
   - Interacción entre componentes
   - **Detalle reciente:** Interfaz de llamadas al sistema y ejecución de GUI en
       Ring 3, con corrección de errores en el stub de syscall (evitar sobrescribir
       argumentos).
     - **Novedades:** Consola gráfica interactiva accesible con F1 y sonido de
       arranque a través del PC speaker (señalización / diagnóstico visual y
       auditivo al iniciar la GUI).
   - Prioridades

### Documentación por Componentes

- **[Bootloader](../boot/README.md)** - FreeLoader adaptado de ReactOS
- **[Kernel](../kernel/README.md)** - Núcleo del sistema operativo
- **[Drivers](../drivers/README.md)** - Controladores de hardware
- **[Librerías](../lib/README.md)** - Runtime libraries y soporte

## Guía de Contribución

### Cómo Contribuir

1. **Fork el Repositorio**
   ```bash
   git clone https://github.com/[tu-usuario]/System_Operative_Edit.git
   ```

2. **Crear una Rama**
   ```bash
   git checkout -b feature/nueva-caracteristica
   ```

3. **Hacer Cambios**
   - Seguir las convenciones de código
   - Documentar cambios
   - Añadir tests si es posible

4. **Commit y Push**
   ```bash
   git add .
   git commit -m "Descripción de cambios"
   git push origin feature/nueva-caracteristica
   ```

5. **Crear Pull Request**
   - Describir los cambios realizados
   - Referenciar issues relacionados
   - Esperar revisión

### Convenciones de Código

#### Estilo C
```c
// Funciones: snake_case
void initialize_kernel(void);

// Macros: UPPER_CASE
#define MAX_BUFFER_SIZE 1024

// Estructuras: snake_case con typedef
typedef struct process_control_block {
    int pid;
    char *name;
} pcb_t;

// Indentación: 4 espacios
if (condition) {
    do_something();
}
```

#### Comentarios
- Comentarios de bloque para funciones
- Comentarios inline para lógica compleja
- Explicar el "por qué", no el "qué"

```c
/*
 * Inicializa el sistema de memoria virtual
 * 
 * Esta función configura las tablas de páginas y habilita
 * la paginación. Debe llamarse después de inicializar
 * el gestor de memoria física.
 */
void init_virtual_memory(void) {
    // Código aquí
}
```

#### Nombres de Archivos
- Minúsculas con guiones bajos: `memory_manager.c`
- Headers con el mismo nombre: `memory_manager.h`
- Un archivo por módulo principal

### Estructura de Commits

```
tipo(alcance): descripción corta

Descripción más detallada si es necesario.
Explicar qué cambió y por qué.

Resolves: #123
```

**Tipos de commit:**
- `feat`: Nueva característica
- `fix`: Corrección de bug
- `docs`: Cambios en documentación
- `style`: Formato, espacios, etc.
- `refactor`: Refactorización de código
- `test`: Añadir o modificar tests
- `chore`: Tareas de mantenimiento

### Testing

Cuando sea posible, incluir tests:
```c
// test_memory.c
void test_memory_allocation(void) {
    void *ptr = kmalloc(100);
    assert(ptr != NULL);
    kfree(ptr);
}
```

## Referencias Importantes

### ReactOS
- **Sitio Web**: https://reactos.org/
- **GitHub**: https://github.com/reactos/reactos
- **Wiki**: https://reactos.org/wiki/
- **Documentación**: https://doxygen.reactos.org/

### Desarrollo de OS
- **OSDev Wiki**: https://wiki.osdev.org/
- **OSDev Forums**: https://forum.osdev.org/
- **Intel Manuals**: https://software.intel.com/content/www/us/en/develop/articles/intel-sdm.html
- **AMD Manuals**: https://developer.amd.com/resources/developer-guides-manuals/

### Herramientas
- **GCC**: https://gcc.gnu.org/
- **NASM**: https://www.nasm.us/
- **QEMU**: https://www.qemu.org/
- **GDB**: https://www.gnu.org/software/gdb/

### Especificaciones
- **x86 Architecture**: Intel/AMD manuals
- **ACPI**: https://uefi.org/specifications
- **PCI**: https://pcisig.com/specifications
- **USB**: https://www.usb.org/documents

## Recursos de Aprendizaje

### Tutoriales
1. **OSDev Bare Bones**: Tutorial básico de OS
2. **Writing a Simple Operating System - from Scratch**: Libro gratuito
3. **The little book about OS development**: Tutorial completo

### Libros Recomendados
- "Operating Systems: Three Easy Pieces" - Free online
- "Modern Operating Systems" by Andrew S. Tanenbaum
- "Operating System Concepts" by Silberschatz
- "Windows Internals" by Mark Russinovich (para entender ReactOS/NT)

### Videos y Cursos
- YouTube: Various OS development series
- Coursera: Operating Systems courses
- edX: Computer Architecture courses

## Glosario

### Términos Comunes

- **HAL**: Hardware Abstraction Layer - Capa de abstracción de hardware
- **IDT**: Interrupt Descriptor Table - Tabla de descriptores de interrupción
- **GDT**: Global Descriptor Table - Tabla de descriptores globales
- **IRQ**: Interrupt Request - Petición de interrupción
- **DMA**: Direct Memory Access - Acceso directo a memoria
- **MMU**: Memory Management Unit - Unidad de gestión de memoria
- **TLB**: Translation Lookaside Buffer - Buffer de traducción de direcciones
- **PCB**: Process Control Block - Bloque de control de proceso
- **IPC**: Inter-Process Communication - Comunicación entre procesos
- **VFS**: Virtual File System - Sistema de archivos virtual
- **ABI**: Application Binary Interface - Interfaz binaria de aplicación
- **API**: Application Programming Interface - Interfaz de programación de aplicaciones

## FAQ

### ¿Por qué usar componentes de ReactOS?
ReactOS es un proyecto maduro con código de calidad que implementa la API de Windows NT. Usar componentes selectos nos ahorra tiempo y proporciona una base sólida.

### ¿Es compatible con Windows?
No necesariamente. Aunque usamos componentes de ReactOS, no es un objetivo principal la compatibilidad binaria con Windows.

### ¿Qué arquitecturas se soportan?
Actualmente: x86 (32-bit)
Futuro: x86-64, ARM

### ¿Puedo usar esto en producción?
No. Este es un proyecto educativo y de desarrollo. No está listo para producción.

### ¿Cómo puedo ayudar?
- Reportar bugs
- Mejorar documentación
- Implementar características
- Escribir tests
- Revisar código

## Contacto

Para preguntas o discusiones:
- **Issues**: Usa el sistema de issues de GitHub
- **Pull Requests**: Para contribuciones de código
- **Discussions**: Para preguntas generales

## Licencia

Todo el código y documentación está bajo GNU GPL v3, compatible con ReactOS.

Ver [LICENSE](../LICENSE) para detalles completos.
