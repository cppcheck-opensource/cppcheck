

typedef int MISRA_2_3_A;
typedef int MISRA_2_3_B;
typedef int MISRA_2_3_VIOLATION; // cppcheck-suppress misra-c2012-2.3

// cppcheck-suppress misra-c2012-2.4
struct misra_2_4_violation_t {
    int x;
};

static inline void misra_5_9_exception(void) {}

void misra_8_7_external(void);

#define MISRA_2_5_OK_1 1
#define MISRA_2_5_OK_2 2
// cppcheck-suppress misra-c2012-2.5
#define MISRA_2_5_VIOLATION 0

// #12362
extern void misra_8_7_compliant( void );

#define MISRA_2_5_OK_3 This