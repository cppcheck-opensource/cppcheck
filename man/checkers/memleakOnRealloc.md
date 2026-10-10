# memleakOnRealloc

**Message**: Common realloc mistake: 'a' nulled but not freed upon failure<br/>
**Category**: Correctness<br/>
**Severity**: Error<br/>
**Language**: C/C++

## Description

The classic `p = realloc(p, newSize);` mistake - if `realloc()` fails, it returns `NULL` without
freeing the original block, but this code has just overwritten `p` with that `NULL`, losing the only
pointer to the original (still allocated) memory.

## Motivation

`realloc()` only frees the original block on success; on failure the original block is untouched and
still needs freeing, but the return value is `NULL`. Assigning the return value straight back onto the
only variable that pointed to the original block throws that pointer away too, permanently leaking the
original allocation whenever `realloc()` fails.

## How to fix

Before:
```cpp
void foo() {
    char *a = (char *)malloc(10);
    a = (char *)realloc(a, 100); // <- original block lost if this fails
    free(a);
}
```

After:
```cpp
void foo() {
    char *a = (char *)malloc(10);
    char *tmp = (char *)realloc(a, 100);
    if (tmp)
        a = tmp;
    free(a);
}
```

## Related checkers

- [memleak.md](memleak.md) - the more general "allocated but never freed" check this one complements.
