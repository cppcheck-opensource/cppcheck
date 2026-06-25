// To test:
// ~/cppcheck/cppcheck --dump misra/misra-test-c11.c --std=c11
// ~/cppcheck/cppcheck --dump -DDUMMY --suppress=uninitvar --inline-suppr misra/misra-test-c11.c --std=c11 --platform=unix64 && python3 ../misra.py -verify misra/misra-test-c11.c.dump

#include <stdbool.h>
#include <stdint.h>

typedef unsigned int UINT_TYPEDEF;
struct struct_with_bitfields
{
  unsigned int a:2; // Compliant
  signed int   b:2; // Compliant
  UINT_TYPEDEF c:2; // Compliant
  int          d:2; // 6.1 - plain int not compliant
  signed long  f:2; // Compliant in c99 or later - explicitly signed integer type
  unsigned int g:1; // Compliant
  signed int   h:1; // 6.2 - signed int with size 1 is not compliant
  uint16_t     i:1; // Compliant
  bool         j:1; // Compliant in C99 or later
};

static void misra6_1_fn(void) {
    // "Use" occurrence should not generate warnings
    struct_with_bitfields s;
    s.h = 61;
}

static void misra_10_3_c11(void) {
    bool b = false;
    bool b0 = 0; // 10.3
    bool b1 = 1; // 10.3
    bool bf = false; // no-warning
    bool bt = true; // no-warning
    b = 0; // 10.3
    b = 1; // 10.3
    b = false; // no-warning
    b = true; // no-warning
}
