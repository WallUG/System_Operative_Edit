# Sistema Operativo Personalizado - ReactOS Custom OS

## Descripción del Proyecto

Este proyecto tiene como objetivo crear un sistema operativo desde cero utilizando componentes selectos de ReactOS. El enfoque es modular, permitiendo integrar solo los componentes necesarios de ReactOS mientras se mantiene una arquitectura simple y comprensible.

## Objetivo

Desarrollar un sistema operativo funcional basado en:
- Arquitectura modular y extensible
- Componentes selectos de ReactOS
- Código limpio y bien documentado
- Facilidad de compilación y desarrollo

## Componentes de ReactOS Planificados

### FreeLoader (Bootloader)
- Bootloader de ReactOS adaptado para nuestro sistema
- Soporte para arranque desde múltiples dispositivos
- Carga del kernel y drivers básicos

### HAL (Hardware Abstraction Layer)
- Capa de abstracción de hardware de ReactOS
- Permite portabilidad entre diferentes arquitecturas
- Manejo de interrupciones y DMA

### Drivers Básicos Seleccionados
- Driver de teclado
- Driver de video VGA
- Driver de disco (IDE/SATA)
- Driver de sistema de archivos

### Librerías Runtime Específicas
- Runtime library (RTL)
- Librerías de soporte del kernel
- Librerías de manejo de memoria

## Requisitos para Compilar

### Herramientas Necesarias
- GCC (GNU Compiler Collection) o MinGW para Windows
- GNU Make
- NASM (Netwide Assembler)
- mkisofs o genisoimage (para crear imágenes ISO)
- QEMU o VirtualBox (para pruebas)

### En Linux/Ubuntu
```bash
sudo apt-get install build-essential nasm gcc make genisoimage qemu-system-x86
```

### En Windows
- Instalar MinGW-w64
- Instalar NASM
- Instalar Make for Windows

## Instrucciones de Compilación

### Compilar todo el proyecto
```bash
make all
```

### Compilar componentes individuales
```bash
make boot      # Compila el bootloader
make kernel    # Compila el kernel
make clean     # Limpia archivos de compilación
```

### Crear imagen ISO (futuro)
```bash
make iso       # Genera una imagen ISO booteable
```

## Estructura del Proyecto

```
/boot           - Bootloader (FreeLoader adaptado)
/kernel         - Núcleo básico del sistema operativo
/drivers        - Controladores de hardware
/lib            - Librerías compartidas y runtime
/docs           - Documentación completa del proyecto
/tools          - Herramientas de compilación y desarrollo
```

## Estado Actual del Proyecto

**Fase Actual:** Estructura inicial del proyecto

### Completado
- [x] Estructura de directorios base
- [x] Documentación inicial
- [x] Sistema de compilación básico (Makefile)

### En Desarrollo
- [ ] Bootloader básico
- [ ] Kernel minimal
- [ ] HAL básico

### Planificado
- [ ] Drivers fundamentales
- [ ] Sistema de archivos
- [ ] Shell simple

Para más detalles, consulta el [Roadmap completo](docs/roadmap.md).

## Licencia

Este proyecto está licenciado bajo GNU General Public License v3.0 (GNU GPL v3), compatible con ReactOS.

Ver el archivo [LICENSE](LICENSE) para más detalles.

## Créditos

### ReactOS
Este proyecto utiliza componentes seleccionados de ReactOS:
- **ReactOS Project:** https://reactos.org/
- **ReactOS GitHub:** https://github.com/reactos/reactos
- **Licencia ReactOS:** GNU GPL v2+

Agradecemos al equipo de ReactOS por su increíble trabajo en crear un sistema operativo de código abierto compatible con Windows NT.

### Componentes Específicos de ReactOS
- FreeLoader - Bootloader
- HAL (Hardware Abstraction Layer)
- Drivers seleccionados
- Runtime libraries

## Documentación

Para más información sobre el proyecto:
- [Arquitectura del Sistema](docs/architecture.md)
- [Roadmap de Desarrollo](docs/roadmap.md)
- [Documentación del Bootloader](boot/README.md)
- [Documentación del Kernel](kernel/README.md)

## Contribución

Las contribuciones son bienvenidas. Por favor:
1. Fork el proyecto
2. Crea una rama para tu feature (`git checkout -b feature/AmazingFeature`)
3. Commit tus cambios (`git commit -m 'Add some AmazingFeature'`)
4. Push a la rama (`git push origin feature/AmazingFeature`)
5. Abre un Pull Request

## Contacto

Para preguntas o sugerencias sobre el proyecto, por favor abre un issue en el repositorio.

---

**Nota:** Este es un proyecto educativo y de desarrollo. No está destinado para uso en producción en su estado actual.
