# Boot Animation Visual Preview - Graphics Mode

This document provides a visual representation of the graphics boot animation sequence in VGA Mode 13h.

## Screen Specifications

- **Resolution**: 320×200 pixels
- **Color Depth**: 8 bits (256 indexed colors)
- **Aspect Ratio**: 16:10 (pixels are slightly wider than tall in Mode 13h)
- **Background**: Black (color index 0)

## Color Palette (Institutional Colors)

```
Index 0:  █ Black          RGB(0, 0, 0)      - Background
Index 1:  █ Blue Dark      RGB(0, 0, 168)    - Logo base
Index 2:  █ Blue Light     RGB(0, 102, 255)  - Logo main
Index 3:  █ Yellow         RGB(255, 255, 0)  - Logo accent
Index 4:  █ Yellow Light   RGB(255, 255, 230) - Highlights
Index 7:  █ Gray Light     RGB(168, 168, 168) - Text
Index 8:  █ Gray Dark      RGB(84, 84, 84)   - Shadows
Index 10: █ Green          RGB(0, 255, 0)    - Progress
Index 15: █ White          RGB(255, 255, 255) - Borders
```

## Animation Sequence

### Frame 1: Initial Black Screen (500ms)
```
┌────────────────────────────────────────────────────────────────────────────┐
│                                                                            │
│                                                                            │
│                                                                            │
│                                                                            │
│                              [BLACK SCREEN]                                │
│                                                                            │
│                                                                            │
│                                                                            │
│                                                                            │
└────────────────────────────────────────────────────────────────────────────┘
    320 pixels wide × 200 pixels tall
    All pixels set to color index 0 (black)
```

### Frame 2: Logo Fade-In Start (Step 1/10)
```
┌────────────────────────────────────────────────────────────────────────────┐
│                                                                            │
│                                                                            │
│                          ┏━━━━━━━━━━━━━━━━┓                               │
│                          ┃                ┃                               │
│                          ┃    [LOGO UG]   ┃  ← Very dim (10% brightness)  │
│                          ┃                ┃                               │
│                          ┗━━━━━━━━━━━━━━━━┛                               │
│                                                                            │
│                                                                            │
└────────────────────────────────────────────────────────────────────────────┘
    Palette values scaled to 10% of target
    Logo barely visible against black background
```

### Frame 3: Logo Fully Visible (1000ms total)
```
┌────────────────────────────────────────────────────────────────────────────┐
│                                                                            │
│                                                                            │
│                       ╔════════════════════════╗                           │
│                       ║                        ║  ← Yellow border          │
│                       ║  ██    ██    ███████   ║                           │
│                       ║  ██    ██   ██         ║  ← Blue/Yellow logo       │
│                       ║  ██    ██   ██  ███    ║                           │
│                       ║  ██    ██   ██    ██   ║                           │
│                       ║   ██████     ███████   ║                           │
│                       ║                        ║                           │
│                       ╚════════════════════════╝                           │
│                                                                            │
│                     Center: (160, 100)                                     │
│                     Size: 100×80 pixels                                    │
└────────────────────────────────────────────────────────────────────────────┘
```

### Detailed Logo Structure
```
    Logo Breakdown (100×80 pixels):

    Outer border: Yellow (index 3), 2 pixels wide
    Background: Blue Light (index 2)
    
    Letter "U":
        Left bar:  6×40 pixels, Yellow
        Right bar: 6×40 pixels, Yellow  
        Bottom:    20×6 pixels, Yellow
        Position:  X=30-50, Y=30-70
    
    Letter "G":
        Left bar:    6×40 pixels, Yellow
        Top bar:     20×6 pixels, Yellow
        Bottom bar:  20×6 pixels, Yellow
        Right bar:   6×20 pixels, Yellow (lower half)
        Mid bar:     10×6 pixels, Yellow
        Position:    X=60-80, Y=30-70
```

### Frame 4: Branding Added (500ms)
```
┌────────────────────────────────────────────────────────────────────────────┐
│                                                                            │
│                                                                            │
│                       ╔════════════════════════╗                           │
│                       ║  ██    ██    ███████   ║                           │
│                       ║  ██    ██   ██         ║                           │
│                       ║  ██    ██   ██  ███    ║                           │
│                       ║  ██    ██   ██    ██   ║                           │
│                       ║   ██████     ███████   ║                           │
│                       ╚════════════════════════╝                           │
│                                                                            │
│                    ────────────────────────────────                        │
│                    ███████████████████████████████  ← Text placeholder    │
│                      ████████████████████████████   ← (White blocks)      │
│                    ────────────────────────────────                        │
│                                                                            │
│                    Line Y: 140-155                                         │
└────────────────────────────────────────────────────────────────────────────┘
```

### Frame 5: Progress Bar - 20% (400ms)
```
┌────────────────────────────────────────────────────────────────────────────┐
│                                                                            │
│                       ╔════════════════════════╗                           │
│                       ║  ██    ██    ███████   ║                           │
│                       ║  ██    ██   ██         ║                           │
│                       ║  ██    ██   ██  ███    ║                           │
│                       ║  ██    ██   ██    ██   ║                           │
│                       ║   ██████     ███████   ║                           │
│                       ╚════════════════════════╝                           │
│                                                                            │
│                    ────────────────────────────────                        │
│                    ███████████████████████████████                         │
│                      ████████████████████████████                          │
│                    ────────────────────────────────                        │
│                                                                            │
│           ┌────────────────────────────────────────────┐                  │
│           │████████────────────────────────────────────│ 20%              │
│           └────────────────────────────────────────────┘                  │
│              ^                                                             │
│              Green (40px) + Gray (160px)                                   │
│              Position: Y=170, X=60-260 (200px wide)                        │
└────────────────────────────────────────────────────────────────────────────┘
```

### Frame 6: Progress Bar - 40% (400ms)
```
           ┌────────────────────────────────────────────┐
           │████████████████────────────────────────────│ 40%
           └────────────────────────────────────────────┘
              Green (80px) + Gray (120px)
```

### Frame 7: Progress Bar - 60% (400ms)
```
           ┌────────────────────────────────────────────┐
           │████████████████████████────────────────────│ 60%
           └────────────────────────────────────────────┘
              Green (120px) + Gray (80px)
```

### Frame 8: Progress Bar - 80% (400ms)
```
           ┌────────────────────────────────────────────┐
           │████████████████████████████████────────────│ 80%
           └────────────────────────────────────────────┘
              Green (160px) + Gray (40px)
```

### Frame 9: Progress Bar - 100% (600ms)
```
┌────────────────────────────────────────────────────────────────────────────┐
│                                                                            │
│                       ╔════════════════════════╗                           │
│                       ║  ██    ██    ███████   ║                           │
│                       ║  ██    ██   ██         ║                           │
│                       ║  ██    ██   ██  ███    ║                           │
│                       ║  ██    ██   ██    ██   ║                           │
│                       ║   ██████     ███████   ║                           │
│                       ╚════════════════════════╝                           │
│                                                                            │
│                    ────────────────────────────────                        │
│                    ███████████████████████████████                         │
│                      ████████████████████████████                          │
│                    ────────────────────────────────                        │
│                                                                            │
│           ┌────────────────────────────────────────────┐                  │
│           │████████████████████████████████████████████│ 100%             │
│           └────────────────────────────────────────────┘                  │
│              Full green bar (200px)                                        │
└────────────────────────────────────────────────────────────────────────────┘
```

### Frame 10: Fade-Out (Step 5/10)
```
┌────────────────────────────────────────────────────────────────────────────┐
│                                                                            │
│                                                                            │
│                       ╔════════════════════════╗                           │
│                       ║  ██    ██    ███████   ║  ← Dimming (50%)         │
│                       ║  ██    ██   ██         ║                           │
│                       ║  ██    ██   ██  ███    ║                           │
│                       ║   ██████     ███████   ║                           │
│                       ╚════════════════════════╝                           │
│                                                                            │
│           ┌────────────────────────────────────────────┐                  │
│           │████████████████████████████████████████████│                  │
│           └────────────────────────────────────────────┘                  │
│              Dimming progress bar                                          │
└────────────────────────────────────────────────────────────────────────────┘
    Palette values scaled to 50% of original
```

### Frame 11: Final Black Screen
```
┌────────────────────────────────────────────────────────────────────────────┐
│                                                                            │
│                                                                            │
│                                                                            │
│                                                                            │
│                              [BLACK SCREEN]                                │
│                                                                            │
│                                                                            │
│                                                                            │
│                                                                            │
└────────────────────────────────────────────────────────────────────────────┘
    Fade complete, ready for text mode transition
```

## Timing Diagram

```
Timeline (Total: ~4.6 seconds)

0.0s  ┬─ [Black Screen]
      │
0.5s  ┼─ Start Fade-In (10 steps)
      │  ├─ Step 1: 10% brightness
      │  ├─ Step 2: 20% brightness
      │  ├─ ...
      │  └─ Step 10: 100% brightness
      │
1.5s  ┼─ Logo Fully Visible
      │
2.0s  ┼─ Branding Added
      │
2.5s  ┼─ Progress: 20% - "Inicializando hardware..."
      │
2.9s  ┼─ Progress: 40% - "Detectando memoria..."
      │
3.3s  ┼─ Progress: 60% - "Inicializando video..."
      │
3.7s  ┼─ Progress: 80% - "Configurando disco..."
      │
4.1s  ┼─ Progress: 100% - "Preparando sistema..."
      │
4.7s  ┼─ Start Fade-Out (10 steps)
      │  ├─ Step 1: 90% brightness
      │  ├─ Step 2: 80% brightness
      │  ├─ ...
      │  └─ Step 10: 0% brightness
      │
5.2s  ┴─ Return to Text Mode
```

## Technical Implementation Notes

### Pixel Coordinates
```
Screen Layout:
┌─────────────────────────┐
│ (0,0)         (319,0)   │  Top
│                         │
│           (160,100)     │  Center
│                         │
│ (0,199)       (319,199) │  Bottom
└─────────────────────────┘
```

### Logo Positioning
- **Center Point**: (160, 100)
- **Logo Top-Left**: (110, 60)
- **Logo Size**: 100×80 pixels
- **Border Width**: 2 pixels (yellow)

### Progress Bar Positioning
- **Y Position**: 170
- **X Start**: 60
- **Width**: 200 pixels
- **Height**: 10 pixels
- **Border**: 1 pixel (white)

### Memory Usage
- **Framebuffer**: 0xA0000 - 0xAFA00 (64000 bytes)
- **Palette Registers**: 0x3C8, 0x3C9
- **Total VRAM Used**: 64000 bytes (320×200)

## Comparison: Graphics vs Text Mode

```
┌─────────────────────────────────────┬─────────────────────────────────────┐
│       Graphics Mode (VGA 13h)       │         Text Mode (Fallback)        │
├─────────────────────────────────────┼─────────────────────────────────────┤
│ Resolution: 320×200 pixels          │ Resolution: 80×25 characters        │
│ Colors: 256 (8-bit indexed)         │ Colors: 16 (4-bit)                  │
│ Logo: Geometric shapes              │ Logo: ASCII art                     │
│ Effects: Palette fade in/out        │ Effects: None (instant display)     │
│ Progress: Filled rectangle          │ Progress: Text characters (=, >, -)│
│ Duration: ~4.6 seconds              │ Duration: ~3.5 seconds              │
│ VRAM: 64 KB                         │ VRAM: 4 KB                          │
│ CPU Usage: Medium (fade effects)    │ CPU Usage: Low                      │
└─────────────────────────────────────┴─────────────────────────────────────┘
```

## Future Enhancements

### Bitmap Font Rendering
```
Current: Text as solid rectangles
Future:  8×8 bitmap font for readable text

Example:
┌────────────────────────────────────────────┐
│  System Operative Edit v0.1                │  ← Real text with bitmap font
│  Universidad de Guayaquil                  │
└────────────────────────────────────────────┘
```

### Logo from BMP File
```
Current: Embedded geometric logo
Future:  Load logo_ug.bmp (200×200, 256 colors)

Would enable:
- Official UG logo with all details
- Better color gradients
- Smoother shapes
- Anti-aliasing (if prepared in BMP)
```

### VESA VBE Mode
```
Current: VGA 13h (320×200)
Future:  VESA modes (640×480, 800×600, 1024×768)

Benefits:
- Higher resolution
- More screen space
- Better looking graphics
- True color (16/24-bit)
```

---

**Note**: This preview uses ASCII art to represent the graphics. The actual implementation uses VGA Mode 13h with direct pixel manipulation and palette control for smooth fade effects and professional appearance.

**See Also**: 
- [docs/BOOT_ANIMATION_GRAPHICS.md](BOOT_ANIMATION_GRAPHICS.md) - Technical documentation
- [boot/bootdata/README.md](../boot/bootdata/README.md) - Logo specifications
