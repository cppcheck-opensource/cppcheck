# invalidPointerCast

**Message**: Casting between float * and double * which have an incompatible binary data representation.<br/>
**Category**: Portability<br/>
**Severity**: Portability<br/>
**Language**: C/C++

## Description

A pointer is cast to a pointer of another type whose values aren't laid out the same way in memory (for
example `float*` to `double*`, or a pointer to a floating-point type cast to/from an integer pointer) -
reading through the new pointer doesn't reinterpret the same bytes the same way on every platform.

## Motivation

Reinterpreting the bytes of one type as if they were another only makes sense when both types share the
same binary layout. Floating-point types in particular can have very different sizes and bit layouts
across platforms, so code that happens to "work" during development can silently misbehave once built
for a different target.

cppcheck flags the cast itself, based only on the two pointer types involved - it doesn't check whether
the resulting pointer is ever actually read through (or otherwise used in a way that depends on the
bytes matching up). Doing so - reading an object through a pointer to an incompatible type - is where
the undefined behaviour actually is; forming the pointer alone is not.

## How to fix

Before:
```cpp
void test() {
    float *f = new float[10];
    delete [] (double*)f; // <- float and double aren't stored the same way
}
```

After:
```cpp
void test() {
    float *f = new float[10];
    delete [] f;
}
```

## Related checkers

- [intToPointerCast.md](intToPointerCast.md) - a different pointer-cast portability issue, about casting
  a plain (non-hex) integer literal directly to a pointer.
