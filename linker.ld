ENTRY(start)

SECTIONS
{
    /* Multiboot header must be within the first 8KB of the file */
    . = 0x00100000;

    .multiboot ALIGN(4) : {
        *(.multiboot)
    }

    /* Text section starts after multiboot, but on same page */
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
}
