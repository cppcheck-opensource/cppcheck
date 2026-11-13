# selfAssignment

**Message**: Redundant assignment of 'x' to itself.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

A variable is assigned to itself (`x = x;`), which has no effect.

## Motivation

An assignment that reads and writes the exact same variable does nothing useful, and is often a
leftover from a refactor or a typo for an assignment that was meant to involve a different variable.

## How to fix

Before:
```cpp
void foo() {
    int x = 1;
    x = x; // <- has no effect
}
```

After:
```cpp
#include <cstdio>
void foo() {
    int x = 1;
    printf("%d", x);
}
```
