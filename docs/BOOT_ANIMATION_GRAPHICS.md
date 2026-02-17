# Boot Animation - Graphics Mode Documentation

## Overview

System_Operative_Edit v0.1 ahora incluye un sistema completo de animación de boot con soporte para modo gráfico VGA, siguiendo la arquitectura de ReactOS para mantener compatibilidad con aplicaciones Win32.

## Arquitectura

### Componentes del Sistema

```
Boot Animation System
├── BootVid Driver (boot/bootvid/)
│   ├── bootvid.c - Driver de video VGA
│   └── bootvid.h - API del driver
│
├── Boot Logo (boot/bootdata/)
│   ├── bootlogo.c - Renderizador de logos
│   ├── bootlogo.h - API del renderizador
│   └── logo_ug.bmp - Logo de la UG (pendiente)
│
├── Hardware Layer (boot/freeldr/arch/i386/)
│   ├── hardware.c - Inicialización de hardware
│   └── hardware.h - API de hardware
│
├── UI Layer (boot/freeldr/ui/)
│   └── animation.c - Lógica de animación gráfica
│
└── Fallback (boot/freeldr/)
    └── boot_animation.c - Animación en modo texto
```

## Modos de Operación

### 1. Modo Gráfico (Primario)

**Especificaciones:**
- **Modo VGA**: 13h (320×200, 256 colores)
- **Memoria de video**: 0xA0000
- **Paleta**: Configuración personalizada con colores UG
- **Efectos**: Fade in/out mediante manipulación de paleta

**Flujo de ejecución:**
```
1. BootMain() inicia
2. VideoInit() - Inicializa modo texto (fallback)
3. HwInitBootGraphics() - Intenta inicializar VGA 13h
4. Si éxito:
   - AnimationGraphicsInit() - Configura paleta
   - AnimationGraphicsShowWelcome() - Ejecuta animación
   - AnimationGraphicsCleanup() - Restaura modo texto
5. Si falla:
   - AnimationInit() - Usa modo texto
   - AnimationShowWelcome() - Animación ASCII
```

### 2. Modo Texto (Fallback)

**Especificaciones:**
- **Modo VGA**: Texto 80×25
- **Colores**: 16 colores estándar VGA
- **Logo**: ASCII art "UG"
- **Barra de progreso**: Caracteres ASCII

**Activación:**
- Automática si falla inicialización de modo gráfico
- Garantiza que el sistema siempre muestre animación

## API del Sistema

### BootVid Driver

#### Inicialización
```c
int BootVidInitialize(void);
```
Cambia a modo VGA 13h. Retorna 1 si exitoso, 0 si falla.

#### Limpieza
```c
void BootVidResetDisplay(void);
```
Restaura el modo de video original.

#### Dibujo Básico
```c
void BootVidClearScreen(unsigned char color);
void BootVidSetPixel(int x, int y, unsigned char color);
void BootVidDrawRect(int x, int y, int width, int height, unsigned char color);
void BootVidDrawRectOutline(int x, int y, int width, int height, unsigned char color);
```

#### Paleta
```c
void BootVidSetPaletteColor(unsigned char index, unsigned char r, unsigned char g, unsigned char b);
void BootVidInitializePalette(void);
```
Los valores RGB van de 0-63 (formato VGA DAC).

#### Efectos
```c
void BootVidFadeScreen(int fade_in, int steps);
```
- `fade_in`: 1 = fade in, 0 = fade out
- `steps`: Número de pasos (más = más suave)

### Boot Logo

```c
int BootLogoRender(const unsigned char *bitmap_data, unsigned int size);
```
Renderiza un logo BMP de 8 bits. Si `bitmap_data` es NULL, usa logo embebido.

```c
void BootLogoDrawProgressBar(int progress, int y);
```
Dibuja barra de progreso (0-100%).

### Hardware Layer

```c
int HwInitBootGraphics(void);
```
Inicializa modo gráfico para boot. Retorna 1 si disponible.

```c
void HwResetGraphics(void);
```
Restaura modo de video original.

### Graphics Animation

```c
int AnimationGraphicsInit(void);
void AnimationGraphicsShowWelcome(void);
void AnimationGraphicsCleanup(void);
```

## Secuencia de Animación

### Modo Gráfico

1. **Pantalla negra** (500ms)
   - Limpia pantalla a color negro

2. **Logo UG con fade-in** (1000ms)
   - Renderiza logo embebido o desde BMP
   - Aplica fade-in de 10 pasos
   - Logo centrado (100×80 píxeles)

3. **Branding** (500ms)
   - Líneas decorativas
   - Rectángulos representando texto
   - "System Operative Edit v0.1"
   - "Universidad de Guayaquil"

4. **Progress bar animado** (2000ms)
   - 5 etapas de inicialización
   - 400ms por etapa
   - Barra de 200 píxeles de ancho
   - Color verde para progreso

5. **Completitud y fade-out** (600ms)
   - Barra al 100%
   - Fade-out de 10 pasos

**Duración total**: ~4.6 segundos

### Modo Texto

1. **Logo ASCII** (800ms)
2. **Inicio de sistema** (500ms)
3. **Progress bar** (1600ms, 4 etapas)
4. **Mensaje de éxito** (600ms)

**Duración total**: ~3.5 segundos

## Paleta de Colores

### Índices de Paleta VGA

| Índice | Color | RGB (0-63) | Uso |
|--------|-------|------------|-----|
| 0 | Negro | (0, 0, 0) | Fondo |
| 1 | Azul Oscuro | (0, 0, 42) | Logo base |
| 2 | Azul Claro | (0, 25, 63) | Logo UG |
| 3 | Amarillo | (63, 63, 0) | Logo/Acento |
| 4 | Amarillo Claro | (63, 63, 42) | Highlights |
| 7 | Gris Claro | (42, 42, 42) | Texto |
| 8 | Gris Oscuro | (21, 21, 21) | Sombras |
| 10 | Verde | (0, 63, 0) | Progress bar |
| 15 | Blanco | (63, 63, 63) | Bordes/texto |

### Conversión RGB

VGA DAC usa valores de 6 bits (0-63) en lugar de 8 bits (0-255):
```
VGA_value = RGB_8bit >> 2
```

## Especificaciones Técnicas

### Modo VGA 13h

- **Resolución**: 320×200 píxeles
- **Profundidad**: 8 bits por píxel (256 colores)
- **Memoria**: 64000 bytes (320 × 200)
- **Direccionamiento**: Lineal desde 0xA0000
- **Paleta**: 256 entradas, 18 bits por color (RGB 6-6-6)

### Registros VGA

| Puerto | Nombre | Uso |
|--------|--------|-----|
| 0x3C8 | PEL Address Write | Seleccionar índice de paleta para escribir |
| 0x3C9 | PEL Data | Escribir componentes R, G, B |
| 0x3C7 | PEL Address Read | Seleccionar índice de paleta para leer |

### Logo Embebido

Dimensiones: 100×80 píxeles
Elementos:
- Rectángulo azul de fondo
- Borde amarillo doble
- Letras "U" y "G" en amarillo
- Centrado en pantalla (110, 60)

## Integración con FreeLoader

### Modificaciones en freeldr.c

```c
void BootMain(void)
{
    VideoInit();  // Modo texto siempre disponible
    
    if (HwInitBootGraphics()) {
        // Modo gráfico disponible
        if (AnimationGraphicsInit()) {
            AnimationGraphicsShowWelcome();
            AnimationGraphicsCleanup();
        }
    } else {
        // Fallback a modo texto
        AnimationInit();
        AnimationShowWelcome();
    }
    
    HwResetGraphics();
    VideoInit();  // Restaurar modo texto
    VideoClearScreen();
    
    // Continuar con boot normal...
}
```

## Compilación

### Makefile

```makefile
FREELDR_OBJS = ... \
               $(BUILD_DIR)/bootvid.o \
               $(BUILD_DIR)/bootlogo.o \
               $(BUILD_DIR)/hardware.o \
               $(BUILD_DIR)/animation.o
```

### Tamaños

- **BootVid**: ~3.5 KB
- **BootLogo**: ~2.0 KB
- **Hardware**: ~0.5 KB
- **Animation**: ~1.5 KB
- **Total overhead**: ~7.5 KB
- **FreeLoader antes**: 8.1 KB
- **FreeLoader después**: 14 KB

## Testing

### En QEMU

```bash
cd /home/runner/work/System_Operative_Edit/System_Operative_Edit
./scripts/run-qemu.sh
```

### Verificación

1. **Modo gráfico funciona**:
   - Logo UG visible en colores
   - Fade in/out suave
   - Barra de progreso verde

2. **Fallback funciona**:
   - Si VGA falla, muestra ASCII art
   - Sistema no se bloquea

3. **Transición correcta**:
   - Después de animación, vuelve a modo texto
   - Info del sistema se muestra correctamente

## Troubleshooting

### Problema: Pantalla negra después de animación
**Solución**: Verificar que `HwResetGraphics()` se llame y que `VideoInit()` re-inicialice modo texto.

### Problema: Logo no se ve / colores incorrectos
**Solución**: Verificar que `BootVidInitializePalette()` se llame antes de renderizar.

### Problema: Sistema se congela en modo gráfico
**Solución**: Asegurar que la CPU soporte modo VGA 13h. Usar fallback a texto.

### Problema: Fade muy lento/rápido
**Solución**: Ajustar el número de `steps` en `BootVidFadeScreen()` y el delay en el loop.

## Optimizaciones Futuras

### Corto Plazo
- [ ] Implementar carga de logo desde BMP real
- [ ] Añadir texto usando bitmap font
- [ ] Optimizar fade usando timer PIT
- [ ] Añadir más efectos visuales

### Mediano Plazo
- [ ] Soporte VESA VBE (resoluciones más altas)
- [ ] True color (16/24 bits)
- [ ] Animación del logo (frames)
- [ ] Efectos de partículas

### Largo Plazo
- [ ] Soporte de múltiples temas
- [ ] Configuración desde archivo
- [ ] Boot splash personalizable
- [ ] Integración con PC Speaker para sonido

## Referencias

- [OSDev VGA](https://wiki.osdev.org/VGA_Hardware)
- [ReactOS BootVid](https://github.com/reactos/reactos/tree/master/boot/freeldr/freeldr/ui)
- [VGA Mode 13h Tutorial](https://en.wikipedia.org/wiki/Mode_13h)
- [BMP File Format](https://en.wikipedia.org/wiki/BMP_file_format)

## Licencia

GPL-3.0 - Ver LICENSE en la raíz del proyecto.

## Contribuciones

Para contribuir al sistema de animación:

1. Mantener compatibilidad con arquitectura ReactOS
2. Asegurar fallback a modo texto siempre funcione
3. Documentar cambios en la paleta de colores
4. Probar en QEMU antes de commit
5. Actualizar esta documentación

## Créditos

- **Universidad de Guayaquil**: Logo e identidad institucional
- **ReactOS Project**: Arquitectura de referencia
- **System_Operative_Edit Team**: Implementación

---

**Última actualización**: Febrero 2024
**Versión**: 1.0
**Estado**: Implementación completa, pendiente testing en hardware real
