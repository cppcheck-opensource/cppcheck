# sizeofVoid and sizeofDereferencedVoidPointer

**Message**: Behaviour of 'sizeof(void)' is not covered by the ISO C standard.<br/>
**Category**: Portability<br/>
**Severity**: Portability<br/>
**Language**: C/C++

## Description

- `sizeofVoid`: `sizeof(void)` is used directly - this isn't defined by the C/C++ standard (only by a
  GNU extension, which defines it as `1`).
- `sizeofDereferencedVoidPointer`: `sizeof(*p)` where `p` is `void*` - the same underlying problem,
  reached by dereferencing a `void*` instead of naming `void` directly.

## Motivation

`void` has no size in standard C/C++ - `sizeof(void)` is only meaningful under a GNU extension (where
it's defined as `1`), so code relying on it is not portable to a strictly-standard-conforming compiler,
even though gcc/clang will happily accept it.

## How to fix

Before:
```cpp
void f(void *p) {
    int s = sizeof(*p); // <- 'p' is void*, so this is sizeof(void)
}
```

After:
```cpp
void f(char *p) {
    int s = sizeof(*p);
}
```

## Related checkers

- [arithOperationsOnVoidPointer.md](arithOperationsOnVoidPointer.md) - another `void*`-specific,
  non-standard construct (pointer arithmetic directly on `void*`).
