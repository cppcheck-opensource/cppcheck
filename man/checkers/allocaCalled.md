# allocaCalled

**Message**: Obsolete function 'alloca' called. In C99 and later it is recommended to use a variable length array instead.<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

`alloca()` is called. cppcheck flags every call unconditionally, regardless of the context it's used
in - it does not check, for example, whether the call is inside a loop.

## Motivation

`alloca()` allocates memory on the stack, but that memory is only freed when the *entire calling
function* returns - not at the end of the block or loop iteration the call happens to be in, the way a
normal local variable would be. This makes `alloca()` inside a loop a common way to exhaust the stack:
each iteration adds another allocation on top of the previous ones, and none of them are released until
the function finally returns, even though the loop itself may look perfectly ordinary.

Unlike `malloc()`, `alloca()` also has no way to report failure - if a request is too large (whether from
one oversized call, or many small ones accumulating in a loop), the result is undefined behaviour
(typically a stack overflow) rather than a clean, checkable error.

## How to fix

Before:
```cpp
#include <alloca.h>
void f(int n, int count) {
    for (int i = 0; i < count; i++) {
        char *buf = alloca(n); // <- each iteration's allocation piles up; none are freed until f() returns
        buf[0] = 0;
    }
}
```

After (C99 and later): a variable length array declared inside the loop body *is* freed at the end of
each iteration, unlike `alloca()`.
```cpp
void f(int n, int count) {
    for (int i = 0; i < count; i++) {
        char buf[n];
        buf[0] = 0;
    }
}
```

Before:
```cpp
#include <alloca.h>
void f(int n) {
    char *buf = alloca(n); // <- obsolete, no error handling if 'n' is too large
}
```

After (C99 and later):
```cpp
void f(int n) {
    char buf[n]; // variable length array
}
```

After (C++11 and later):
```cpp
#include <array>
void f() {
    std::array<char, 128> buf; // fixed-size, or use a dynamically allocated container
}
```
