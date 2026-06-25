#include "stdint.h"
#include "stdio.h"

#define PRINT_DEF(d, f) \
    fprintf(stdout, ";"#d "=%"#f, d)

#define PRINT_DEF_N(d1, d2, f) \
    do { \
        PRINT_DEF(d1 ## 8 ## d2, f); \
        PRINT_DEF(d1 ## 16 ## d2, f); \
        PRINT_DEF(d1 ## 32 ## d2, f); \
        PRINT_DEF(d1 ## 64 ## d2, l ## f); \
    } while (0)

// TODO: fix all format specifiers
int main(void)
{
    PRINT_DEF(INTMAX_MIN, ld);
    PRINT_DEF(INTMAX_MAX, ld);
    PRINT_DEF(UINTMAX_MAX, lu);
    PRINT_DEF_N(INT, _MIN, d);
    PRINT_DEF_N(INT, _MAX, d);
    PRINT_DEF_N(UINT, _MAX, u);
    PRINT_DEF_N(INT_LEAST, _MIN, d);
    PRINT_DEF_N(INT_LEAST, _MAX, d);
    PRINT_DEF_N(UINT_LEAST, _MAX, u);
    PRINT_DEF_N(INT_FAST, _MIN, d);
    PRINT_DEF_N(INT_FAST, _MAX, d);
    PRINT_DEF_N(UINT_FAST, _MAX, u);
    PRINT_DEF(INTPTR_MIN, ld);
    PRINT_DEF(INTPTR_MAX, ld);
    PRINT_DEF(UINTPTR_MAX, lu);
    PRINT_DEF(SIZE_MAX, lu);
    PRINT_DEF(PTRDIFF_MIN, ld);
    PRINT_DEF(PTRDIFF_MAX, ld);
    PRINT_DEF(SIG_ATOMIC_MIN, d);
    PRINT_DEF(SIG_ATOMIC_MAX, d);
    PRINT_DEF(WCHAR_MIN, d);
    PRINT_DEF(WCHAR_MAX, d);
    PRINT_DEF(WINT_MIN, d);
    PRINT_DEF(WINT_MAX, d);

    return 0;
}
