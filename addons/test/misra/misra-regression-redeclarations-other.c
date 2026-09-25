#include "misra-regression-redeclarations.h"

int two_definitions = 10;

static int read_other(void)
{
    return elsewhere;
}
