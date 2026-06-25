// Test with command:
// ./cppcheck --enable=information --enable=style --addon=misra --inline-suppr addons/test/misra/misra-ctu-*-test.c

#include "misra-ctu-test.h"

extern MISRA_2_3_B  misra_2_3_b;

x = MISRA_2_5_OK_2;

// cppcheck-suppress misra-c2012-5.6
typedef int MISRA_5_6_VIOLATION;
static MISRA_5_6_VIOLATION misra_5_6_x;

// cppcheck-suppress misra-c2012-5.7
struct misra_5_7_violation_t {
    int x;  // cppcheck-suppress unusedStructMember
};
static misra_5_7_violation_t misra_5_7_use_type_2;

// #11443 - FP
static struct
{ // no warning
    uint16_t x;  // cppcheck-suppress unusedStructMember
} misra_5_7_false_positive_2;

// cppcheck-suppress misra-c2012-5.8
static int misra_5_8_var1;
// cppcheck-suppress misra-c2012-8.4
// cppcheck-suppress misra-c2012-5.8
void misra_5_8_f(void) {
    // cppcheck-suppress [misra-c2012-5.8, unusedVariable]
    char misra_5_8_var2;
}

// cppcheck-suppress misra-c2012-5.9
static int misra_5_9_count;
// cppcheck-suppress misra-c2012-5.9
static void misra_5_8_foo(void) {}

// cppcheck-suppress misra-c2012-8.5
extern int misra_8_5;

// cppcheck-suppress misra-c2012-8.4
// cppcheck-suppress misra-c2012-8.6
int32_t misra_8_6 = 2;
int32_t misra_8_6_1;
// cppcheck-suppress misra-c2012-8.7
int32_t misra_8_6_1 = 2;
// cppcheck-suppress misra-c2012-8.4
// cppcheck-suppress misra-c2012-8.7
void misra_8_7(void) {}
static void misra_8_7_caller(void) { 
    misra_8_7(); 
    misra_8_7_external();
}

// #12362
typedef void(*misra_8_7_func_ptr)( void );
static const misra_8_7_func_ptr ptrs[] = { misra_8_7_compliant, NULL };