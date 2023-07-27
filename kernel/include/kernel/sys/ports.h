// Файл ports.h

#ifndef _KERNEL_SYS_PORTS_H
#define _KERNEL_SYS_PORTS_H

#include <stdint.h>

#define qemu_log(M, ...) __com_formatString(0x3f8,"[LOG] (%s:%s:%d) " M "\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)

typedef unsigned int 	u32int;
typedef          int	s32int;
typedef unsigned short	u16int;
typedef          short  s16int;
typedef unsigned char	u8int;
typedef          char	s8int;

// Функция для отправки байта данных на порт ввода-вывода
void outb(uint16_t port, uint8_t data);

// Функция для получения байта данных с порта ввода-вывода
uint8_t inb(uint16_t port);

// Функция для получения слова (16 бит) данных с порта ввода-вывода
uint16_t inw(uint16_t port);

void outw(uint16_t port, uint16_t data);


#endif
