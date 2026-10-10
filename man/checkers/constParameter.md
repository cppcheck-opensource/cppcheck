# constParameter

**Message**: Parameter 'x' can be declared as const<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

An array-typed function parameter is never used to modify its contents, so it could be declared as an
array of `const` elements.

## Motivation

A missing `const` hides a guarantee the compiler could otherwise enforce and readers could otherwise
rely on: that the function only reads the array, never modifies it.

## How to fix

Before:
```cpp
void f(int n, int v[42]) { // <- 'v' is only read
    int j = 0;
    for (int i = 0; i < n; ++i) {
        j += 1;
        if (j == 1) {}
    }
}
```

After:
```cpp
void f(int n, const int v[42]) {
    int j = 0;
    for (int i = 0; i < n; ++i) {
        j += 1;
        if (j == 1) {}
    }
}
```

## Related checkers

- [constVariable.md](constVariable.md) - the same idea, for a local array variable instead of a parameter.
- [constParameterReference.md](constParameterReference.md) - the reference-parameter equivalent.
- [constParameterPointer.md](constParameterPointer.md) - the pointer-parameter equivalent.
- [constParameterCallback.md](constParameterCallback.md) - the same idea, but for a parameter of a
  function used as a callback.
