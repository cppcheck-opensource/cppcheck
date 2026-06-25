// Test with command:
// ./cppcheck --enable=information --enable=style --addon=misra --inline-suppr addons/test/misra/misra-ctu-*-test.c

#include "misra-ctu-test.h"

extern MISRA_2_3_A  misra_2_3_a;

x = MISRA_2_5_OK_1;

// cppcheck-suppress misra-c2012-20.5
#undef MISRA_2_5_OK_3

// cppcheck-suppress misra-c2012-2.3
// cppcheck-suppress misra-c2012-5.6
typedef int MISRA_5_6_VIOLATION;

// cppcheck-suppress misra-c2012-5.7
struct misra_5_7_violation_t {
    int x;  // cppcheck-suppress unusedStructMember
};
static misra_5_7_violation_t misra_5_7_use_type_1;

// #11443 - FP
static struct
{ // no warning
    uint16_t x;  // cppcheck-suppress unusedStructMember
} misra_5_7_false_positive_1;

// cppcheck-suppress misra-c2012-8.4
// cppcheck-suppress misra-c2012-5.8
int misra_5_8_var1;
// cppcheck-suppress misra-c2012-8.4
// cppcheck-suppress misra-c2012-5.8
int misra_5_8_var2;
// cppcheck-suppress misra-c2012-5.8
static void misra_5_8_f(void) {}


// cppcheck-suppress misra-c2012-5.9
static int misra_5_9_count;
// cppcheck-suppress misra-c2012-5.9
static void misra_5_8_foo(void) {}

// cppcheck-suppress misra-c2012-8.5
extern int misra_8_5;

// cppcheck-suppress misra-c2012-8.4
// cppcheck-suppress misra-c2012-8.6
int32_t misra_8_6 = 1;

void misra_8_7_external(void) {}

// #12362
void misra_8_7_compliant( void ){}
static void misra_8_7_call(void) { misra_8_7_compliant(); }