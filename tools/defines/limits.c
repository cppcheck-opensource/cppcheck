#include "limits.h"
#include "stdio.h"

#define PRINT_DEF(d, f) \
    fprintf(stdout, ";"#d "=%"#f, d)

int main(void)
{
    PRINT_DEF(CHAR_BIT, d);
    PRINT_DEF(SCHAR_MIN, d);
    PRINT_DEF(SCHAR_MAX, d);
    PRINT_DEF(UCHAR_MAX, d);
    PRINT_DEF(CHAR_MIN, d);
    PRINT_DEF(CHAR_MAX, d);
    PRINT_DEF(MB_LEN_MAX, d);
    PRINT_DEF(SHRT_MIN, d);
    PRINT_DEF(SHRT_MAX, d);
    PRINT_DEF(USHRT_MAX, d);
    PRINT_DEF(INT_MIN, d);
    PRINT_DEF(INT_MAX, d);
    PRINT_DEF(UINT_MAX, u);
    PRINT_DEF(LONG_MIN, ld);
    PRINT_DEF(LONG_MAX, ld);
    PRINT_DEF(ULONG_MAX, ld);
#if (__STDC_VERSION__ >= 199901L) || (__cplusplus >= 201103L)
    PRINT_DEF(LLONG_MIN, lld);
    PRINT_DEF(LLONG_MAX, lld);
    PRINT_DEF(ULLONG_MAX, llu);
#endif

    return 0;
}
