ENTRY(start)

SECTIONS
{
    /* Kernel cargado en 1MB */
    . = 1M;

    .text ALIGN(4K) : {
        *(.multiboot)   /* header multiboot va primero */
        *(.text)
    }

    .rodata ALIGN(4K) : {
        *(.rodata)
    }

    .data ALIGN(4K) : {
        *(.data)
    }

    .bss ALIGN(4K) : {
        _bss_start = .;
        *(COMMON)
        *(.bss)
        _bss_end = .;
    }

    /* Stack al final, alineado, bien separado del kernel */
    . = ALIGN(4K);
    _stack_bottom = .;
    . += 0x8000;    /* 32 KB de stack */
    _stack_top = .;
}
