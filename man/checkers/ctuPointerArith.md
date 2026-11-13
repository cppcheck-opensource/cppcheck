# ctuPointerArith

**Message**: Pointer arithmetic overflow; 'p' buffer size is 12<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C/C++

## Description

The same idea as [pointerOutOfBounds.md](pointerOutOfBounds.md), but found by cppcheck's whole-program
("cross translation unit") analysis, which follows a buffer across function calls that the normal,
per-function analysis doesn't always chain together. Because of this, the same bug can sometimes be
reported twice - once under the plain ID and once under this `ctu`-prefixed one - for the same line.

## Motivation

Pointer arithmetic that goes out of bounds is undefined behaviour whether it's visible directly inside
one function or only becomes apparent by following a buffer through a call into another function - the
whole-program analysis exists to catch the cases a single function's view can't.

## How to fix

Before:
```cpp
void dostuff(int *p) { int x = *(p + 10); }
int main() {
    int arr[3];
    dostuff(arr); // <- 'arr' is too small for what dostuff() does with it
}
```

After:
```cpp
void dostuff(int *p) { int x = *(p + 10); }
int main() {
    int arr[11];
    dostuff(arr);
}
```

## Related checkers

- [pointerOutOfBounds.md](pointerOutOfBounds.md) - the per-function version of this check.
- [ctuArrayIndex.md](ctuArrayIndex.md) - the whole-program array-indexing equivalent.
