# Guía de compilación detallada

## Pasos detallados

### 1. Preparar entorno

Verificar versiones:
```bash
gcc --version
nasm -version
cmake --version
qemu-system-i386 --version
```

### 2. Compilar desde cero

```bash
# Limpiar builds anteriores
rm -rf build/

# Compilar
./scripts/build.sh
```

### 3. Solución de problemas comunes

**Error: "gcc: error: unrecognized command line option '-m32'"**
- Solución: Instalar gcc-multilib: `sudo apt-get install gcc-multilib`

**Error: "nasm: command not found"**
- Solución: Instalar NASM: `sudo apt-get install nasm`

**Error al crear ISO**
- Verificar que grub-mkrescue esté instalado
- Instalar: `sudo apt-get install grub-pc-bin xorriso`

### 4. Debugging

Ejecutar con GDB:
```bash
# Terminal 1
qemu-system-i386 -kernel build/kernel.elf -s -S

# Terminal 2
gdb build/kernel.elf
(gdb) target remote localhost:1234
(gdb) continue
```
