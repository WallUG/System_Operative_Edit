# Boot Animation Visual Preview

This document shows what users will see during the boot animation sequence.

## Screen 1: UG Logo Display (Duration: ~800ms)

```
================================================================================


                    ========================================
                             _    _    _____  
                            | |  | |  / ____| 
                            | |  | | | |  __  
                            | |  | | | | |_ | 
                            | |__| | | |__| | 
                             \____/   \_____| 
                    ========================================

                       UNIVERSIDAD DE GUAYAQUIL
                       System Operative Edit v0.1
                       Edicion Universidad de Guayaquil
                    ========================================



                        Iniciando sistema...


  Inicializando hardware...
  [=>--------------------------------------]  0%



================================================================================
```

## Screen 2: Progress - Step 1 (Duration: 400ms)

```
                    ========================================
                             _    _    _____  
                            | |  | |  / ____| 
                            | |  | | | |  __  
                            | |  | | | | |_ | 
                            | |__| | | |__| | 
                             \____/   \_____| 
                    ========================================

                       UNIVERSIDAD DE GUAYAQUIL
                       System Operative Edit v0.1
                       Edicion Universidad de Guayaquil
                    ========================================



                        Iniciando sistema...


  Inicializando hardware...
  [========>-------------------------------]  20%
```

## Screen 3: Progress - Step 2 (Duration: 400ms)

```
                    ========================================
                             _    _    _____  
                            | |  | |  / ____| 
                            | |  | | | |  __  
                            | |  | | | | |_ | 
                            | |__| | | |__| | 
                             \____/   \_____| 
                    ========================================

                       UNIVERSIDAD DE GUAYAQUIL
                       System Operative Edit v0.1
                       Edicion Universidad de Guayaquil
                    ========================================



                        Iniciando sistema...


  Detectando memoria...
  [================>-----------------------]  40%
```

## Screen 4: Progress - Step 3 (Duration: 400ms)

```
                    ========================================
                             _    _    _____  
                            | |  | |  / ____| 
                            | |  | | | |  __  
                            | |  | | | | |_ | 
                            | |__| | | |__| | 
                             \____/   \_____| 
                    ========================================

                       UNIVERSIDAD DE GUAYAQUIL
                       System Operative Edit v0.1
                       Edicion Universidad de Guayaquil
                    ========================================



                        Iniciando sistema...


  Inicializando video...
  [========================>---------------]  60%
```

## Screen 5: Progress - Step 4 (Duration: 400ms)

```
                    ========================================
                             _    _    _____  
                            | |  | |  / ____| 
                            | |  | | | |  __  
                            | |  | | | | |_ | 
                            | |__| | | |__| | 
                             \____/   \_____| 
                    ========================================

                       UNIVERSIDAD DE GUAYAQUIL
                       System Operative Edit v0.1
                       Edicion Universidad de Guayaquil
                    ========================================



                        Iniciando sistema...


  Configurando disco...
  [================================>-------]  80%
```

## Screen 6: Progress - Complete (Duration: 400ms)

```
                    ========================================
                             _    _    _____  
                            | |  | |  / ____| 
                            | |  | | | |  __  
                            | |  | | | | |_ | 
                            | |__| | | |__| | 
                             \____/   \_____| 
                    ========================================

                       UNIVERSIDAD DE GUAYAQUIL
                       System Operative Edit v0.1
                       Edicion Universidad de Guayaquil
                    ========================================



                        Iniciando sistema...


  Listo!
  [========================================]  100%

                    Sistema iniciado correctamente!
```

## Screen 7: Transition to System Info (Duration: 800ms)

After the animation completes, the screen clears and shows the traditional FreeLoader banner:

```
================================================================================

  ========================================
      FreeLoader - System Operative Edit
  ========================================

Informacion del Sistema:
------------------------

  Unidad de arranque: 0x80
  Particion: 0

Memoria Detectada:
------------------

  Memoria baja:  640 KB
  Memoria alta:  65536 KB
  Total:         66176 KB (64 MB)

Estado del Bootloader:
----------------------

  [OK] Video inicializado
  [OK] Memoria detectada
  [OK] Disco inicializado
  [OK] Sistema listo

[... continues with more system information ...]
================================================================================
```

## Color Scheme

Throughout the animation, the following color scheme is used:

- **Logo "UG" letters**: Bright Yellow (VGA color 0x0E)
- **Separator lines**: Light Cyan (VGA color 0x0B)
- **University name**: White (VGA color 0x0F)
- **Descriptive text**: Light Gray (VGA color 0x07)
- **Progress bar (filled)**: Light Green (VGA color 0x0A)
- **Progress bar (active indicator '>')**: Yellow (VGA color 0x0E)
- **Progress bar (empty)**: Dark Gray (VGA color 0x08)
- **Success messages**: Light Green (VGA color 0x0A)

These colors represent the institutional colors of Universidad de Guayaquil (blue and yellow) while maintaining good readability on VGA text mode displays.

## Total Animation Time

- Logo display: 800ms
- Progress step 1: 400ms
- Progress step 2: 400ms
- Progress step 3: 400ms
- Progress step 4: 400ms
- Progress step 5: 400ms
- Success message: 600ms
- **Total**: ~3.4 seconds

The animation provides visual feedback without significantly impacting boot time, creating a professional and branded boot experience for Universidad de Guayaquil.
