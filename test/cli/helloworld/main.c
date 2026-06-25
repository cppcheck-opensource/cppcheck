#include <stdio.h>

int main(void) {
    (void)printf("Hello world!\n");
    int x = 3 / 0; (void)x; // ERROR
    return 0;
}

#ifdef SOME_CONFIG
void foo();
#endif
