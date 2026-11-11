#include "board.h"

void gpio_set(unsigned int pin, unsigned int mask)
{
    GPIOA_ODR |= mask;
}
