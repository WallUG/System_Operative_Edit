ENTRY(start)

SECTIONS
{
    /* Multiboot header must be within the first 8KB of the file */
    . = 0x00100000;

    /* Multiboot header requires 4-byte alignment per spec */
    .multiboot ALIGN(4) : {
        *(.multiboot)
    }

    /* Text section starts after multiboot with 16-byte alignment for performance */
    .text ALIGN(16) : {
        *(.text)
    }

    .rodata ALIGN(4K) : {
        *(.rodata)
    }

    .data ALIGN(4K) : {
        *(.data)
    }

    .bss ALIGN(4K) : {
        *(COMMON)
        *(.bss)
    }

    /* Simbolo que marca el fin del kernel image.
     * memory.c puede usarlo para inicializar el heap justo
     * despues del kernel, sin hardcodear una direccion fija. */
    _kernel_end = .;
}
