/* Linker script para el kernel
 * Define la disposición de memoria del kernel
 */

/* Punto de entrada del kernel */
ENTRY(_start)

/* Secciones de memoria */
SECTIONS
{
    /* El kernel se carga en 1MB (dirección física) */
    . = 1M;

    /* Sección de código */
    .text ALIGN(4K) : {
        *(.text)
    }

    /* Sección de datos de solo lectura */
    .rodata ALIGN(4K) : {
        *(.rodata)
    }

    /* Sección de datos inicializados */
    .data ALIGN(4K) : {
        *(.data)
    }

    /* Sección de datos no inicializados (BSS) */
    .bss ALIGN(4K) : {
        *(COMMON)
        *(.bss)
    }
}
