# ctuArrayIndex

**Message**: Array index out of bounds; 'p' buffer size is 4 and it is accessed at offset 40.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C/C++

## Description

The same idea as [arrayIndexOutOfBounds.md](arrayIndexOutOfBounds.md), but found by cppcheck's
whole-program ("cross translation unit") analysis, which follows a buffer across function calls that
the normal, per-function analysis doesn't always chain together. Because of this, the same bug can
sometimes be reported twice - once under the plain ID and once under this `ctu`-prefixed one - for the
same line.

## Motivation

An out-of-bounds array access is undefined behaviour whether it's visible directly inside one function
or only becomes apparent by following a buffer through a call into another function - the whole-program
analysis exists to catch the cases a single function's view can't.

## How to fix

Before:
```cpp
void f(char *p) {
    p[9] = 0;
}
void g() {
    char buf[5];
    f(buf); // <- 'buf' is too small for what f() does with it
}
```

After:
```cpp
void f(char *p) {
    p[9] = 0;
}
void g() {
    char buf[10];
    f(buf);
}
```

## Related checkers

- [arrayIndexOutOfBounds.md](arrayIndexOutOfBounds.md) - the per-function version of this check.
- [ctuPointerArith.md](ctuPointerArith.md) - the whole-program pointer-arithmetic equivalent.
