# memleak and resourceLeak

**Message**: Memory leak: p<br/>
**Category**: Correctness<br/>
**Severity**: Error<br/>
**Language**: C/C++

## Description

A locally-allocated variable is allocated (`malloc`/`new`/`fopen`/and other functions described the
same way in the library configuration), but no path frees/releases it before it goes out of scope, is
reassigned, or the function returns.

- `memleak`: the allocation is memory (`malloc`, `new`, ...).
- `resourceLeak`: the allocation is some other kind of resource (a file handle from `fopen`, ...).

## Motivation

A memory or resource leak wastes memory/handles until the process exits, which can degrade or crash
long-running programs. This is easy for a human reviewer to miss, especially across multiple `if`/`else`
branches, but mechanical enough for cppcheck to track precisely in straightforward code. cppcheck only
tracks an allocation through straight-line code: once a loop or `goto` appears anywhere in the function,
it stops checking that function entirely rather than risk a wrong guess, so its silence on a function
with a loop in it isn't proof the code has no leak.

## How to fix

Before:
```cpp
void f() {
    char *p = malloc(10); // <- memleak: never freed
}
```

After:
```cpp
void f() {
    char *p = malloc(10);
    free(p);
}
```

## False positives to be aware of

- **A false positive is possible when a variable is freed only through a reference alias to it.** The
  checker doesn't always recognize that freeing the alias also frees the original variable, and can
  report a leak that doesn't actually exist:
  ```cpp
  void f() {
      char *p;
      char *&ref = p;
      p = malloc(10);
      free(ref); // frees 'p', but is reported as "Memory leak: p" anyway
  }
  ```

## Related checkers

- [mismatchAllocDealloc.md](mismatchAllocDealloc.md) - for when the allocation *is* freed, but with the
  wrong deallocation function.
- [doubleFree.md](doubleFree.md) / [deallocuse.md](deallocuse.md) / [deallocret.md](deallocret.md) - the
  related mistakes of freeing something twice, or using it afterwards, once it has been freed.
