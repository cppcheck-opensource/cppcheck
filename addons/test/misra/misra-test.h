#ifndef MISRA_TEST_H
#define MISRA_TEST_H
struct misra_h_s { int foo; };
bool test(char *a); // OK
int misra_8_2_no_fp(int a);
void misra_8_4_bar(void);
// #12978
typedef struct m8_4_stErrorDef
{
    uint8_t ubReturnVal;
} m8_4_stErrorDef;
extern const m8_4_stErrorDef * m8_4_pubTestPointer;
#endif // MISRA_TEST_H
