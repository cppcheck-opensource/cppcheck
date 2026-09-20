# variableScope

**Message**: The scope of the variable 'x' can be reduced.<br/>
**Category**: Readability<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

A variable is declared in an outer scope, it can be declared in an inner scope.

## Motivation

It is common practice, especially in C++ code, to declare variables in inner scope when
possible.

The motivation is to make the code more readable.

Opinions about what is more readable can differ so make your own decisions - these warnings
can be easily suppressed.

## How to fix

Before:
```cpp
#include <cstdio>
void f(bool x) {
    int i = 0; // <- 'i' is only used inside the 'if' below
    if (x) {
        i = 10;
        printf("%d\n", i);
    }
}
```

After:
```cpp
#include <cstdio>
void f(bool x) {
    if (x) {
        int i = 10;
        printf("%d\n", i);
    }
}
```
