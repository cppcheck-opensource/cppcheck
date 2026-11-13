# deallocret

**Message**: Returning/dereferencing 'p' after it is deallocated / released<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C/C++

## Description

A locally-allocated variable is returned, or dereferenced as part of a `return`, after it has already
been freed.

## Motivation

Returning a pointer after it's been freed hands the caller a pointer that's already invalid - using a
pointer after it's been freed is undefined behaviour regardless of which statement does it.

## How to fix

Before:
```cpp
int* f() {
    int *p = malloc(10);
    free(p);
    return p; // <- deallocret
}
```

After:
```cpp
int* f() {
    int *p = malloc(10);
    return p;
}
```

## Related checkers

- [deallocuse.md](deallocuse.md) - the same idea, but the freed variable is dereferenced by an ordinary
  statement rather than a `return`.
