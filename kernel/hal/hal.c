#include "hal.h"

void pit_set_frequency(uint32_t freq_hz)
{
    /* Reprograma el PIT canal 0 para la frecuencia deseada. Divisor = 1193182 / freq. */
    if (freq_hz == 0) return;
    uint16_t divisor = (uint16_t)(1193182 / freq_hz);

    /* Modo 3 (square wave generator), canal 0, acceso por bytes LSB/MSB */
    outb(0x43, 0x36);
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)(divisor >> 8));
}

void hal_init(void)
{
    /* Las interrupciones ya fueron habilitadas despues de idt_init() en main.c.
     * NO llamar cli aqui — matar las interrupciones despues del IDT hace que
     * el mouse PS/2 y el idle loop (hlt) no funcionen, lo que desactiva la CPU
     * en VMware/VirtualBox con el mensaje 'guest disabled the CPU'. */

    /* Ajustar el timer PIT a 100 Hz para obtener quantums más finos y mejorar
     * la responsividad de la GUI; la constante de quantum en el scheduler puede
     * ajustarse después en kernel/proc/scheduler.h si es necesario. */
    pit_set_frequency(100);

    /* HAL inicializado correctamente */
}

/* I/O port operations */
uint8_t inb(uint16_t port)
{
    uint8_t result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

uint16_t inw(uint16_t port)
{
    uint16_t result;
    __asm__ volatile("inw %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

void outw(uint16_t port, uint16_t value)
{
    __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

uint32_t inl(uint16_t port)
{
    uint32_t result;
    __asm__ volatile("inl %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

void outl(uint16_t port, uint32_t value)
{
    __asm__ volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}

/* CPU operations */
void cpu_halt(void)
{
    __asm__ volatile("hlt");
}

void cpu_disable_interrupts(void)
{
    __asm__ volatile("cli");
}

void cpu_enable_interrupts(void)
{
    __asm__ volatile("sti");
}
