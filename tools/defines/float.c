#include "float.h"
#include "stdio.h"

#define PRINT_DEF(d, f) \
    fprintf(stdout, ";"#d "=%"#f, d)

int main(void)
{
    PRINT_DEF(FLT_RADIX, d);
    PRINT_DEF(FLT_MANT_DIG, d);
    PRINT_DEF(DBL_MANT_DIG, d);
    PRINT_DEF(LDBL_MANT_DIG, d);
    PRINT_DEF(FLT_DIG, d);
    PRINT_DEF(DBL_DIG, d);
    PRINT_DEF(LDBL_DIG, d);
    PRINT_DEF(FLT_MIN_EXP, d);
    PRINT_DEF(DBL_MIN_EXP, d);
    PRINT_DEF(LDBL_MIN_EXP, d);
    PRINT_DEF(FLT_MIN_10_EXP, d);
    PRINT_DEF(DBL_MIN_10_EXP, d);
    PRINT_DEF(LDBL_MIN_10_EXP, d);
    PRINT_DEF(FLT_MAX_EXP, d);
    PRINT_DEF(DBL_MAX_EXP, d);
    PRINT_DEF(LDBL_MAX_EXP, d);
    PRINT_DEF(FLT_MAX_10_EXP, d);
    PRINT_DEF(DBL_MAX_10_EXP, d);
    PRINT_DEF(LDBL_MAX_10_EXP, d);
    PRINT_DEF(FLT_MAX, f); // TODO: float-to-double
    PRINT_DEF(DBL_MAX, f);
    PRINT_DEF(LDBL_MAX, Lf);
    PRINT_DEF(FLT_EPSILON, f); // TODO: float-to-double
    PRINT_DEF(DBL_EPSILON, f);
    PRINT_DEF(LDBL_EPSILON, Lf);
    PRINT_DEF(FLT_MIN, f); // TODO: float-to-double
    PRINT_DEF(DBL_MIN, f);
    PRINT_DEF(LDBL_MIN, Lf);
#if (__STDC_VERSION__ >= 199901L) || (__cplusplus >= 201103L)
    PRINT_DEF(FLT_EVAL_METHOD, d);
    PRINT_DEF(DECIMAL_DIG, d);
#endif

    return 0;
}
