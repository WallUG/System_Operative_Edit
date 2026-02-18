; idt_load.asm
; Carga el IDT y define el handler por defecto
BITS 32
section .text
global idt_load
global _default_exception_handler

idt_load:
    mov eax, [esp+4]
    lidt [eax]
    ret

_default_exception_handler:
    cli
    hlt
    jmp _default_exception_handler
