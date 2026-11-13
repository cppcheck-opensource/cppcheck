# leakReturnValNotUsed

**Message**: Return value of allocation function 'malloc' is not stored.<br/>
**Category**: Correctness<br/>
**Severity**: Error<br/>
**Language**: C/C++

## Description

The result of an allocating call (`malloc(10);`, `new Foo;`) is discarded immediately - there was never
anywhere to store it, so it's an instant, guaranteed leak.

## Motivation

An allocating call's whole purpose is to hand back a pointer to the new memory/object - discarding that
return value immediately means the memory is allocated and then instantly unreachable, with no way for
anything to ever free it. Unlike most leaks, which depend on some later code path forgetting to free
something, this one is a leak the moment the line runs.

## How to fix

Before:
```cpp
void f() {
    malloc(10); // <- return value discarded
}
```

After:
```cpp
void f() {
    char* p = (char*)malloc(10);
    free(p);
}
```

## Related checkers

- [memleak.md](memleak.md) - the more general "allocated but never freed" check, for allocations that
  are stored somewhere but still never freed.
