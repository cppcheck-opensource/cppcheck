# pointerOutOfBounds and pointerOutOfBoundsCond

**Message**: Undefined behaviour, pointer arithmetic 'a+20' is out of bounds.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Portability/Warning<br/>
**Language**: C/C++

## Description

Pointer arithmetic (`p + n`, `p - n`, `p += n`, `p++`, ...) produces a pointer that lands outside the
array/buffer `p` points into:

- `pointerOutOfBounds`: cppcheck knows for certain the result is out of bounds. This is undefined
  behaviour even before the resulting pointer is dereferenced.
- `pointerOutOfBoundsCond`: the out-of-bounds result only holds on one branch of a condition checked
  elsewhere - so either that condition is redundant, or this arithmetic is a bug.

## Motivation

Forming a pointer that lands outside the bounds of the array it points into is undefined behaviour in
C/C++, even if that pointer is never dereferenced - the language only guarantees pointer arithmetic
stays valid up to one-past-the-end of an array. Compilers are allowed to (and do) optimize based on
this assumption, which can make the resulting bug manifest in surprising, hard-to-reproduce ways.

## How to fix

Before:
```cpp
void f() {
    int a[10];
    int *p = a + 20; // <- pointerOutOfBounds
}
```

After:
```cpp
void f() {
    int a[20];
    int *p = a + 19;
}
```

## Related checkers

- [arrayIndexOutOfBounds.md](arrayIndexOutOfBounds.md) - the array-indexing equivalent of this check.
- [ctuPointerArith.md](ctuPointerArith.md) - the same idea, found by cppcheck's whole-program analysis
  across function calls.
