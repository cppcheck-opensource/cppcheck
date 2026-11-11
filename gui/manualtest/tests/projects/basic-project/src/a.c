#include "util.h"

int first(void)
{
    int *p = 0;
    return *p;
}

int second(void)
{
    int *q = 0;
    return *q;
}

int main(void)
{
    return first() + second() + helper();
}
