# deallocuse

**Message**: Dereferencing 'p' after it is deallocated / released<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C/C++

## Description

A locally-allocated variable is dereferenced after it has already been freed.

## Motivation

Using a pointer after it's been freed is undefined behaviour, and one of the more common causes of
memory corruption - the memory often still looks intact for a while afterwards, which is exactly what
makes this bug easy to miss in testing.

## How to fix

Before:
```cpp
void f() {
    int *ptr = new int;
    delete(ptr);
    *ptr = 0; // <- deallocuse
}
```

After:
```cpp
void f() {
    int *ptr = new int;
    *ptr = 0;
    delete(ptr);
}
```

## Related checkers

- [deallocret.md](deallocret.md) - the same idea, but the freed variable is dereferenced as part of a
  `return` statement.
- [doubleFree.md](doubleFree.md) - the related mistake of freeing the same variable again, rather than
  dereferencing it.
