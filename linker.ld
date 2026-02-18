ENTRY(start)

SECTIONS
{
    . = 0x00100000;

    .multiboot ALIGN(4) : {
        *(.multiboot)
    }

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
        _bss_start = .;
        *(COMMON)
        *(.bss)
        _bss_end = .;
    }

    /* Stack al final del kernel, 32KB, bien separado */
    . = ALIGN(4K);
    _stack_bottom = .;
    . += 0x8000;
    _stack_top = .;
}
