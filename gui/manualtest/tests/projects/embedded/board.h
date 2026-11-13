#ifndef BOARD_H
#define BOARD_H

#define GPIOA_ODR (*(volatile unsigned int *)0x40020014U)
#define UART_DR   (*(volatile unsigned int *)0x40011004U)

void gpio_set(unsigned int pin, unsigned int mask);
void uart_putc(char c);

#endif
