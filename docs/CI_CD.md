# CI/CD - GitHub Actions

## Workflow de Compilación Automática

El proyecto utiliza GitHub Actions para compilar automáticamente el sistema operativo en cada push.

### Archivos
- `.github/workflows/build.yml` - Workflow principal

### Proceso
1. Instalar dependencias (gcc, nasm, cmake, grub, qemu)
2. Compilar bootloader (boot/)
3. Compilar kernel (CMake)
4. Crear ISO booteable
5. Probar en QEMU
6. Generar artifacts descargables

### Artifacts
Los siguientes artifacts se generan y están disponibles por 90 días:
- `system-operative-edit-iso` - Imagen ISO booteable completa
- `kernel-binary` - Kernel ELF standalone
- `freeldr-bootloader` - FreeLoader compilado

### Cómo Descargar
1. Ve a la pestaña "Actions" del repositorio
2. Selecciona el workflow run más reciente con estado exitoso (indicado con una marca de verificación verde)
3. Scroll hasta "Artifacts" al final de la página
4. Click para descargar el ZIP
5. Extrae el ISO y úsalo

### Testing Local vs CI

**CI (GitHub Actions)**:
- Usa Ubuntu latest
- Compilación limpia cada vez
- Artifacts automáticos
- No requiere setup local

**Local**:
- Requiere instalar dependencias
- Más rápido para iteración
- Ver errores en tiempo real

### Triggers
- Push a `main`
- Pull Requests a `main`
- Manual (workflow_dispatch)
- Tags `v*` → Release automático

### Releases
Cuando se crea un tag con formato `v*`:
```bash
git tag v0.1.0
git push origin v0.1.0
```
El workflow automáticamente:
- Crea un GitHub Release
- Adjunta el ISO
- Añade notas de release

### Status Badge
Agrega al README.md:
```markdown
![Build Status](https://github.com/WallUG/System_Operative_Edit/workflows/Build%20System%20Operative%20Edit/badge.svg)
```
