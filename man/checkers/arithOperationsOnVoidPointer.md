# arithOperationsOnVoidPointer

**Message**: 'x' is of type 'void *'. When using void pointers in calculations, the behaviour is undefined.<br/>
**Category**: Portability<br/>
**Severity**: Portability<br/>
**Language**: C/C++

## Description

Pointer arithmetic (`+`, `-`, `++`, `--`, `+=`, `-=`) is performed directly on a `void*`, which isn't
standard C/C++ (also only a GNU extension).

## Motivation

Pointer arithmetic advances a pointer by a number of elements, each `sizeof(*p)` bytes - but `void` has
no defined size in standard C/C++, so what "one element" means for a `void*` isn't standard either. Code
relying on this compiles under gcc/clang's extension (which defines it as if `sizeof(void)` were 1) but
isn't portable to a strictly-standard-conforming compiler, and is technically undefined behaviour under
the plain standard rather than the GNU extension cppcheck's own message refers to.

## How to fix

Before:
```cpp
void f(void* p) {
    p = p + 1; // <- arithmetic directly on a void*
}
```

After:
```cpp
void f(char* p) {
    p = p + 1;
}
```

## Related checkers

- [sizeofVoid.md](sizeofVoid.md) - another `void`-specific, non-standard construct (`sizeof(void)` /
  `sizeof(*voidPointer)`).
