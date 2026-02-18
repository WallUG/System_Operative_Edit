; boot.asm - Multiboot entry point
BITS 32

MULTIBOOT_MAGIC       equ 0x1BADB002
MULTIBOOT_PAGE_ALIGN  equ 1 << 0
MULTIBOOT_MEMORY_INFO equ 1 << 1
MULTIBOOT_FLAGS       equ MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO
MULTIBOOT_CHECKSUM    equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

section .multiboot
align 4
    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd MULTIBOOT_CHECKSUM

section .text
global start
extern kernel_main
extern _stack_top

start:
    cli                 ; deshabilitar interrupciones

    ; IMPORTANTE: guardar EAX y EBX ANTES de cualquier otra cosa
    ; EAX = multiboot magic number (puesto por GRUB o QEMU)
    ; EBX = puntero a multiboot_info_t
    mov edi, eax        ; guardar magic en EDI (no destruido por mov seg)
    mov esi, ebx        ; guardar mbi    en ESI (no destruido por mov seg)

    ; Configurar stack
    mov esp, _stack_top
    xor ebp, ebp

    ; Limpiar segmentos usando ax=0 SIN tocar eax completo
    ; (ya guardamos magic en edi)
    mov ax, 0x0000
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Pasar argumentos a kernel_main(magic, mbi)
    push esi            ; arg2: multiboot_info_t* (EBX original)
    push edi            ; arg1: magic number      (EAX original)

    call kernel_main

.hang:
    cli
    hlt
    jmp .hang

section .note.GNU-stack noalloc noexec nowrite progbits
