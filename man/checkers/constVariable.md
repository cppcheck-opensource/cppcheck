# constVariable

**Message**: Variable 'x' can be declared as const<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

A local array variable is never used to modify its contents, so it could be declared as an array of
`const` elements.

## Motivation

A missing `const` hides a guarantee the compiler could otherwise enforce and readers could otherwise
rely on: that this array, once initialized, is never written to again.

## How to fix

Before:
```cpp
int f() {
    static int i[1] = {}; // <- 'i' is only read
    return i[0];
}
```

After:
```cpp
int f() {
    static const int i[1] = {};
    return i[0];
}
```

## Related checkers

- [constParameter.md](constParameter.md) - the same idea, for an array-typed function parameter instead
  of a local variable.
- [constVariableReference.md](constVariableReference.md) - the reference-variable equivalent.
- [constVariablePointer.md](constVariablePointer.md) - the pointer-variable equivalent.
