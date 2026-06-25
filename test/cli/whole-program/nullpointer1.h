#include "nullpointer1_1.h"

template<typename T>
void f(T* p) {
    if (sizeof(T) == 4)
        p = nullptr;
    g(p);
}
