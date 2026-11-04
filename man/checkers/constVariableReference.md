# constVariableReference

**Message**: Variable 'x' can be declared as reference to const<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C++

## Description

A local reference variable is never used to modify what it refers to, so it could be declared as a
reference to `const`.

## Motivation

A missing `const` hides a guarantee the compiler could otherwise enforce and readers could otherwise
rely on: that this reference is only ever used to read the object it refers to.

## How to fix

Before:
```cpp
#include <cstdio>
void f(int i) {
    int &j = i; // <- 'j' is only read
    printf("%d\n", j);
}
```

After:
```cpp
#include <cstdio>
void f(int i) {
    const int &j = i;
    printf("%d\n", j);
}
```

## Related checkers

- [constParameterReference.md](constParameterReference.md) - the same idea, for a reference parameter
  instead of a local variable.
- [constVariable.md](constVariable.md) - the array-variable equivalent.
- [constVariablePointer.md](constVariablePointer.md) - the pointer-variable equivalent.
