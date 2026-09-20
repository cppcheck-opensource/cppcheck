# pointerSize and sizeofDivisionMemfunc

**Message**: Size of pointer 'x' used instead of size of its data.<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

`sizeof(ptr)` (the size of the pointer itself) is used where the size of what it points to was clearly
intended, as an argument to `malloc`/`calloc`/`memset`/`memcpy`/`memmove`/`strncpy`/`strncmp`/`strncat`:

- `pointerSize`: `sizeof(ptr)` is passed directly as (or as part of) the size argument.
- `sizeofDivisionMemfunc`: the size argument is computed by *dividing* by `sizeof(ptr)`, when
  multiplying was clearly intended.

## Motivation

`sizeof(ptr)` is the fixed size of the pointer itself (typically 4 or 8 bytes), not the size of the
buffer it points to. Using it to size a `malloc`/`memset`/`memcpy`-family call is a classic copy-paste
bug: it usually still compiles and often still "sort of works" for small, coincidentally-sized cases,
while silently under- or over-sizing the real operation. The line itself is just a miscalculated size -
the actual undefined behaviour comes later, if something then reads or writes the buffer assuming it's
the size that was intended rather than the (usually too small) size it actually got.

## How to fix

Before:
```cpp
void f() {
    int *x = (int*)malloc(sizeof(x)); // <- size of the pointer, not what it points to
}
```

After:
```cpp
void f() {
    int *x = (int*)malloc(sizeof(*x));
}
```

Before:
```cpp
void f(char* dst, char* src, int size) {
    memcpy(dst, src, size / sizeof(dst)); // <- dividing by the pointer's size, not the data's
}
```

After:
```cpp
void f(char* dst, char* src, int size) {
    memcpy(dst, src, size);
}
```

## Related checkers

- [multiplySizeof.md](multiplySizeof.md) - a related but more general `sizeof(a) * sizeof(b)` /
  `sizeof(a) / sizeof(b)` mistake, not specific to these memory functions.
