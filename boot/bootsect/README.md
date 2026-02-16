# Boot Sectors

Este directorio contiene los boot sectors del bootloader del sistema operativo.

## Origen

Estos boot sectors provienen del proyecto **ReactOS** (https://reactos.org), específicamente del directorio `boot/freeldr/bootsect/` del repositorio https://github.com/reactos/reactos.

## Licencia

Los boot sectors están licenciados bajo GPL-2.0 o GPL-2.0+, compatible con la licencia del proyecto ReactOS. Todos los headers de copyright originales se mantienen intactos.

## Archivos

### fat.S - Boot Sector FAT12/16
- **Autor original**: Brian Palmer
- **Copyright**: (c) 1998, 2001, 2002 Brian Palmer
- **Propósito**: Boot sector para sistemas de archivos FAT12 y FAT16
- **Funcionalidad**:
  - Busca el archivo `FREELDR.SYS` en el directorio raíz
  - Carga el primer sector de FREELDR.SYS en la dirección 0000:F800
  - Configura la pila en 0000:7BF2
  - Carga la tabla FAT completa en memoria en 7000:0000 para acelerar el arranque desde disquete

### fat32.S - Boot Sector FAT32
- **Autor original**: Brian Palmer
- **Copyright**: Ver archivo para detalles
- **Propósito**: Boot sector para sistemas de archivos FAT32
- **Funcionalidad**:
  - Similar a fat.S pero adaptado para FAT32
  - Maneja las estructuras extendidas de FAT32
  - Soporta volúmenes FAT32 con clusters grandes

### isoboot.S - Boot Sector ISO-9660 (CD-ROM)
- **Autores originales**: H. Peter Anvin, Michael K. Ter Louw, Eric Kohl, y otros (ver archivo)
- **Copyright**: Múltiples autores (1994-2017)
- **Propósito**: Boot sector para arrancar desde CD-ROM usando El Torito
- **Funcionalidad**:
  - Basado en ISOLINUX
  - Implementa el estándar El Torito en modo "no emulation"
  - Busca y carga FREELDR.SYS desde CD-ROM ISO-9660

## Funcionamiento General

Todos los boot sectors siguen un patrón común:

1. **Carga inicial**: El BIOS carga el boot sector (512 bytes para FAT, 2048 para ISO) en la dirección 0x7C00
2. **Inicialización**: El boot sector configura los registros de segmento, la pila y el entorno
3. **Búsqueda**: Busca el archivo `FREELDR.SYS` en el sistema de archivos
4. **Carga**: Carga FREELDR.SYS en memoria (típicamente en 0000:F800)
5. **Transferencia**: Salta a FREELDR.SYS para continuar el proceso de arranque

## Modificaciones

**IMPORTANTE**: Estos boot sectors NO deben modificarse. Son código probado y funcional de ReactOS. Cualquier cambio podría causar que el sistema no arranque.

Si se necesitan modificaciones, se debe:
1. Hacer una copia del archivo original
2. Documentar claramente los cambios
3. Probar exhaustivamente en hardware real y emuladores

## Créditos

Agradecemos enormemente al proyecto ReactOS y a todos sus contribuidores por el desarrollo de estos boot sectors de alta calidad:

- **ReactOS Project**: https://reactos.org
- **Brian Palmer**: Autor principal de los boot sectors FAT
- **H. Peter Anvin**: Autor de ISOLINUX (base de isoboot.S)
- **Comunidad ReactOS**: Por el mantenimiento y mejoras continuas

## Referencias

- Repositorio ReactOS: https://github.com/reactos/reactos
- Documentación ReactOS: https://reactos.org/wiki
- El Torito Specification: Para arranque desde CD-ROM
- FAT File System: Especificación Microsoft

## Compilación

Los boot sectors se compilan con el GNU Assembler (as):

```bash
as --32 -o fat.o fat.S
ld -m elf_i386 -T linker.ld -o fat.bin fat.o
```

Ver `../Makefile` para el proceso de compilación completo.
