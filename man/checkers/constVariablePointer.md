# constVariablePointer

**Message**: Variable 'x' can be declared as pointer to const<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

A local pointer variable is never used to modify what it points to, so it could be declared as a
pointer to `const`.

## Motivation

A missing `const` hides a guarantee the compiler could otherwise enforce and readers could otherwise
rely on: that this pointer is only ever used to read the data it points to.

## How to fix

Before:
```cpp
#include <cstdio>
void f() {
    int x = 5;
    int *tm = &x; // <- 'tm' is only used to read '*tm'
    printf("%d\n", *tm);
}
```

After:
```cpp
#include <cstdio>
void f() {
    int x = 5;
    const int *tm = &x;
    printf("%d\n", *tm);
}
```

## Related checkers

- [constParameterPointer.md](constParameterPointer.md) - the same idea, for a pointer parameter instead
  of a local variable.
- [constVariable.md](constVariable.md) - the array-variable equivalent.
- [constVariableReference.md](constVariableReference.md) - the reference-variable equivalent.
