; gdt_flush.asm - GDT definida y cargada completamente en ASM
; Garantiza que los valores son correctos independientemente del compilador
BITS 32
section .text
global gdt_flush
global gdt_install

; ─── Tabla GDT definida directamente en ASM ───────────────────────────────────
section .data
align 8

gdt_start:
    ; Entrada 0: NULL descriptor (obligatorio)
    dq 0x0000000000000000

    ; Entrada 1: Kernel Code  base=0, limit=4GB, ring0, ejecutable
    ; Flags: presente, ring0, tipo codigo, ejecutable, legible
    ; Gran=1 (4KB), 32bit, limit=0xFFFFF
    dq 0x00CF9A000000FFFF

    ; Entrada 2: Kernel Data  base=0, limit=4GB, ring0, datos
    dq 0x00CF92000000FFFF

    ; Entrada 3: User Code  ring3
    dq 0x00CFFA000000FFFF

    ; Entrada 4: User Data  ring3
    dq 0x00CFF2000000FFFF
gdt_end:

gdt_ptr:
    dw gdt_end - gdt_start - 1   ; límite = 5*8-1 = 39 = 0x27
    dd gdt_start                  ; dirección base de la tabla

section .text

; void gdt_flush(uint32_t ignored) - el argumento ya no se usa,
; cargamos la tabla definida arriba directamente
gdt_flush:
    lgdt [gdt_ptr]          ; cargar el puntero GDT definido en .data

    mov ax, 0x10            ; selector kernel data (entrada 2)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    push dword 0x08         ; selector kernel code (entrada 1)
    push dword .flush       ; dirección de retorno
    retf                    ; far return recarga CS

.flush:
    ret
