# leakNoVarFunctionCall

**Message**: Allocation with malloc, strcpy doesn't release it.<br/>
**Category**: Correctness<br/>
**Severity**: Error<br/>
**Language**: C/C++

## Description

The result of an allocating call is passed directly into another function that is not known to store,
free, or otherwise take ownership of it (`strcpy(dst, strdup(src))`) - once that outer call returns,
nothing holds onto the allocated memory.

## Motivation

Passing an allocation straight into another call without ever storing it in a variable only works if
that outer call takes ownership (stores or frees the pointer itself). Most functions - like `strcpy()`,
which only reads through the pointer it's given - don't; once the statement finishes, the allocated
memory is unreachable and leaked. This is mainly reliable for calls to well-known library functions;
cppcheck does not reliably work out ownership for a user-defined function, even one whose body is
visible and plainly doesn't take ownership, so it stays quiet there rather than guess.

## How to fix

Before:
```cpp
void f() {
    strcpy(a, strdup(p)); // <- strcpy() doesn't free its 2nd argument
}
```

After:
```cpp
void f() {
    char* tmp = strdup(p);
    strcpy(a, tmp);
    free(tmp);
}
```

## Related checkers

- [leakReturnValNotUsed.md](leakReturnValNotUsed.md) - the simpler case where the allocation's result
  isn't passed anywhere at all, just discarded.
