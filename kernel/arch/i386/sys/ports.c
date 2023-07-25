// Файл ports.c

#include <kernel/kernel.h>

// Функция для отправки байта данных на порт ввода-вывода
void outb(uint16_t port, uint8_t data)
{
    // Используем инлайн-ассемблер для выполнения инструкции outb
    asm volatile ("outb %0, %1" : : "a"(data), "Nd"(port));
}

// Функция для получения байта данных с порта ввода-вывода
uint8_t inb(uint16_t port)
{
    // Используем инлайн-ассемблер для выполнения инструкции inb
    uint8_t result;
    asm volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

// Функция для получения слова (16 бит) данных с порта ввода-вывода
uint16_t inw(uint16_t port)
{
    // Используем инлайн-ассемблер для выполнения инструкции inw
    uint16_t result;
    asm volatile ("inw %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}


void outw(uint16_t port, uint16_t data) {
    asm("out %%ax, %%dx" : : "a" (data), "d" (port));
}

void irq_disable() {
    __asm__( "cli" );
}

void irq_enable() {
    __asm__( "sti" );
}
