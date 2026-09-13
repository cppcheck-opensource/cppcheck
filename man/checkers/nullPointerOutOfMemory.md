# nullPointerOutOfMemory and nullPointerOutOfResources

**Message**: Null pointer dereference<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

- `nullPointerOutOfMemory`: a pointer came from a memory-allocation function that can fail and return
  null (`malloc`, `new` in some configurations, ...) and is used without checking for that failure.
- `nullPointerOutOfResources`: the same idea for a handle from a resource-allocating function that can
  fail (`fopen`, ...) rather than a memory allocator.

## Motivation

`malloc()` and similar functions are documented to return `NULL` on failure - assuming they always
succeed means that, under memory or resource pressure, the very next dereference crashes instead of the
program handling the failure gracefully.

## How to fix

Before:
```cpp
void f() {
    int *p = malloc(10);
    *p = 1; // <- malloc() can return NULL
    free(p);
}
```

After:
```cpp
void f() {
    int *p = malloc(10);
    if (p) {
        *p = 1;
        free(p);
    }
}
```

## Related checkers

- [nullPointer.md](nullPointer.md) - the general null-dereference check.
- [ctunullpointer.md](ctunullpointer.md) - the whole-program-analysis counterpart, which has its own `ctunullpointerOutOfMemory`/`ctunullpointerOutOfResources` variants.
