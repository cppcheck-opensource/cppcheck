# nullPointerArithmetic

**Message**: Pointer addition with NULL pointer.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C/C++

## Description

Pointer arithmetic (`p + n`, `p - n`, `p++`, `--p`, ...) is performed on a pointer that is null (or
could be). Adding to or subtracting from a null pointer is undefined behaviour even though no memory is
actually touched.

## Motivation

It's easy to assume pointer arithmetic is only dangerous once the result is dereferenced, but the C/C++
standards make forming an out-of-bounds pointer - including any arithmetic on a null pointer - undefined
behaviour in its own right, regardless of whether the result is ever used.

## How to fix

Before:
```cpp
void foo(char *s) {
    char *p = s + 20; // <- if foo() is ever called with a null 's'
}
void bar() { foo(0); }
```

After:
```cpp
void foo(char *s) {
    if (s)
        char *p = s + 20;
}
void bar() { foo(0); }
```

## Related checkers

- [nullPointer.md](nullPointer.md) - the direct-dereference equivalent of this check.
- [nullPointerArithmeticRedundantCheck.md](nullPointerArithmeticRedundantCheck.md) - the same idea, refined by a nearby null check.
- [nullPointerArithmeticOutOfMemory.md](nullPointerArithmeticOutOfMemory.md) - the same idea for a pointer from a failable allocation function.
