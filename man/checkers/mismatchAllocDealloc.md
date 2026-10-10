# mismatchAllocDealloc

**Message**: Mismatching allocation and deallocation: f<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C/C++

## Description

A locally-allocated variable is freed with a function that doesn't match how it was allocated (for
example `new[]` freed with `delete`, or `fopen` freed with `free`).

## Motivation

Freeing something with the wrong function is undefined behaviour: `new[]`/`delete` mismatches can
corrupt the heap, and freeing a `FILE*` with `free()` instead of `fclose()` skips flushing/closing the
underlying file descriptor. This is easy for a human reviewer to miss, but mechanical enough for
cppcheck to track precisely in straightforward code.

## How to fix

Before:
```cpp
void f() {
    FILE *f = fopen(fname, mode);
    free(f); // <- mismatchAllocDealloc: fopen() must be matched with fclose()
}
```

After:
```cpp
void f() {
    FILE *f = fopen(fname, mode);
    fclose(f);
}
```

## Related checkers

- [memleak.md](memleak.md) - for the related, simpler case of an allocation that is never freed at all.
