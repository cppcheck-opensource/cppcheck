# variableScope

**Message**: The scope of the variable 'x' can be reduced.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

A variable is declared in an outer scope than where it's actually used - narrowing its declaration to
the innermost block that needs it makes the code easier to follow.

## Motivation

A variable declared too early forces a reader to track its lifetime across code that doesn't use it,
and makes it unclear at a glance which block actually depends on it.

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
