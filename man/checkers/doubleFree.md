# doubleFree

**Message**: Memory pointed to by 'p' is freed twice.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C/C++

## Description

The same locally-allocated variable is freed/released twice.

## Motivation

Freeing the same memory twice is undefined behaviour, and a common way memory-allocator corruption bugs
are introduced - it's easy to miss when the two frees are far apart, or on different branches that both
happen to execute. cppcheck only tracks an allocation through straight-line code: as soon as a loop or
`goto` appears anywhere in the function, it stops checking that function rather than risk a wrong guess,
so silence on a function with a loop in it isn't proof the code is free of double frees.

## How to fix

Before:
```cpp
void f() {
    char *p = malloc(10);
    free(p);
    free(p); // <- doubleFree
}
```

After:
```cpp
void f() {
    char *p = malloc(10);
    free(p);
}
```

## Related checkers

- [deallocuse.md](deallocuse.md) - the related mistake of dereferencing (rather than freeing again)
  something that's already been freed.
- [memleak.md](memleak.md) - the opposite mistake: an allocation that's never freed at all.
