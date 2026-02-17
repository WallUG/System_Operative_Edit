# Universidad de Guayaquil Logo BMP

Este directorio debe contener el logo de la Universidad de Guayaquil en formato BMP para ser usado durante el boot.

## Especificaciones del Logo

### Formato Requerido
- **Tipo**: BMP (Windows Bitmap)
- **Profundidad de color**: 8 bits (256 colores indexados)
- **Dimensiones recomendadas**: 200x200 píxeles
- **Compresión**: Sin compresión (BI_RGB)
- **Paleta**: Colores institucionales de la UG

### Colores Institucionales UG
- **Azul oscuro**: RGB(0, 0, 168) - Color principal
- **Azul claro**: RGB(0, 102, 255) - Color secundario
- **Amarillo**: RGB(255, 255, 0) - Color de acento
- **Blanco**: RGB(255, 255, 255) - Fondo/texto

## Archivos

### logo_ug.bmp (pendiente)
El logo oficial de la Universidad de Guayaquil en formato BMP de 256 colores.

**Cómo crear el logo:**

1. Obtener el logo oficial de la Universidad de Guayaquil en formato vectorial o de alta resolución
2. Usar un editor de imágenes (GIMP, Photoshop, etc.) para:
   - Redimensionar a 200x200 píxeles
   - Convertir a modo de color indexado (256 colores)
   - Usar la paleta de colores institucionales
   - Guardar como BMP sin compresión

3. Comando GIMP (línea de comandos):
```bash
gimp -i -b '
  (let* ((image (car (gimp-file-load RUN-NONINTERACTIVE "logo_original.png" "logo_original.png")))
         (drawable (car (gimp-image-get-active-layer image))))
    (gimp-image-scale image 200 200)
    (gimp-image-convert-indexed image CONVERT-DITHER-NONE CONVERT-PALETTE-CUSTOM 256 FALSE FALSE "")
    (file-bmp-save RUN-NONINTERACTIVE image drawable "logo_ug.bmp" "logo_ug.bmp" 0)
    (gimp-quit 0))
' -b '(gimp-quit 0)'
```

4. Comando ImageMagick:
```bash
convert logo_original.png -resize 200x200 -colors 256 BMP3:logo_ug.bmp
```

## Fallback Logo Embebido

Si no se proporciona el archivo `logo_ug.bmp`, el sistema usará un logo embebido generado por código que dibuja las letras "UG" usando rectángulos en modo gráfico VGA.

El logo embebido se renderiza en `boot/bootdata/bootlogo.c` en la función `draw_embedded_logo()`.

## Integración

El logo se carga y renderiza automáticamente durante el boot:

1. El bootloader inicializa el modo gráfico VGA (320x200, 256 colores)
2. `BootLogoRender()` intenta cargar `logo_ug.bmp`
3. Si falla la carga, usa el logo embebido
4. El logo se muestra centrado en la pantalla
5. Se aplica un efecto de fade-in usando manipulación de paleta

## Pruebas

Para probar con un logo real:

1. Crear el archivo BMP según las especificaciones
2. Copiarlo a `boot/bootdata/logo_ug.bmp`
3. Modificar el build system para incluir el BMP en la imagen ISO
4. Actualizar `BootLogoRender()` para cargar el archivo desde el filesystem

## Notas Técnicas

### Estructura del BMP
```c
// File Header (14 bytes)
typedef struct {
    uint16_t bfType;       // 'BM' (0x4D42)
    uint32_t bfSize;       // Tamaño del archivo
    uint16_t bfReserved1;  // 0
    uint16_t bfReserved2;  // 0
    uint32_t bfOffBits;    // Offset a datos de imagen
} BMP_FILE_HEADER;

// Info Header (40 bytes)
typedef struct {
    uint32_t biSize;          // 40
    int32_t  biWidth;         // 200
    int32_t  biHeight;        // 200 (positivo = bottom-up)
    uint16_t biPlanes;        // 1
    uint16_t biBitCount;      // 8 (256 colores)
    uint32_t biCompression;   // 0 (BI_RGB)
    uint32_t biSizeImage;     // 0 o tamaño real
    int32_t  biXPelsPerMeter; // 0
    int32_t  biYPelsPerMeter; // 0
    uint32_t biClrUsed;       // 256
    uint32_t biClrImportant;  // 0
} BMP_INFO_HEADER;

// Paleta (1024 bytes = 256 * 4)
// Cada entrada: [Blue, Green, Red, Reserved]

// Datos de imagen (40000 bytes = 200 * 200)
// Cada byte es un índice en la paleta (0-255)
// Las filas están alineadas a 4 bytes
```

### Tamaño Total
- File Header: 14 bytes
- Info Header: 40 bytes
- Paleta: 1024 bytes (256 colores × 4 bytes)
- Datos: 40000 bytes (200 × 200 píxeles)
- **Total**: ~41 KB

## Referencias

- [BMP File Format Specification](https://en.wikipedia.org/wiki/BMP_file_format)
- [Universidad de Guayaquil - Identidad Visual](https://www.ug.edu.ec/)
