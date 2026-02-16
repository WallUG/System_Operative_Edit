; boot.asm - Multiboot bootloader
; This bootloader is compatible with GRUB and follows the Multiboot specification

; Multiboot header constants
MULTIBOOT_MAGIC         equ 0x1BADB002
MULTIBOOT_PAGE_ALIGN    equ 1 << 0
MULTIBOOT_MEMORY_INFO   equ 1 << 1
MULTIBOOT_FLAGS         equ MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO
MULTIBOOT_CHECKSUM      equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

; Declare multiboot header
section .multiboot
align 4
    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd MULTIBOOT_CHECKSUM

; Reserve stack space
section .bss
align 16
stack_bottom:
    resb 16384      ; 16 KB stack
stack_top:

; Entry point
section .text
global start
extern kernel_main

start:
    ; Setup stack
    mov esp, stack_top

    ; Disable interrupts
    cli

    ; Push multiboot information
    ; EAX contains magic value
    ; EBX contains address of multiboot info structure
    push ebx
    push eax

    ; Call kernel main function
    call kernel_main

    ; If kernel_main returns (it shouldn't), hang the system
.hang:
    cli
    hlt
    jmp .hang
