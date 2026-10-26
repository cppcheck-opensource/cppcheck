#include "board.h"

void uart_putc(char c)
{
    UART_DR = (unsigned int)c;
}
