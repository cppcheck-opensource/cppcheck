# ctuuninitvar

**Message**: Using argument p that points at uninitialized variable x<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C/C++

## Description

The same idea as [uninitvar.md](uninitvar.md), but found by cppcheck's whole-program ("cross
translation unit") analysis, which follows an uninitialized value across function calls that the
normal, per-function analysis doesn't always chain together. Because of this, the same bug can
sometimes be reported twice - once as `uninitvar` and once as `ctuuninitvar` - for the same line.

## Motivation

Reading an uninitialized variable is undefined behaviour whether it's visible directly inside one
function or only becomes apparent by following a pointer through a call into another function - the
whole-program analysis exists to catch the cases a single function's view can't.

## How to fix

Before:
```cpp
void f(int *p) {
    a = *p; // <- if f() is ever called with an uninitialized argument
}
int main() {
    int x;
    f(&x);
}
```

After:
```cpp
void f(int *p) {
    a = *p;
}
int main() {
    int x = 0;
    f(&x);
}
```

## Related checkers

- [uninitvar.md](uninitvar.md) - the per-function version of this check.
