#include "board.h"

int main(void)
{
    gpio_set(5U, 0x20U);
    uart_putc('A');
    for (;;)
    {
    }
}
