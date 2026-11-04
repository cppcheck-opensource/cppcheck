# pointerAdditionResultNotNull

**Message**: Comparison is wrong. Result of 'ptr+1' can't be 0 unless there is pointer overflow, and pointer overflow is undefined behaviour.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

Pointer arithmetic is compared against `NULL`/`0`, which can only be true through undefined-behaviour
pointer overflow.

## Motivation

Adding a positive offset to a valid, non-null pointer can never legitimately produce a null pointer - the
only way the comparison could be true is via pointer overflow, which is itself undefined behaviour. A
check written this way doesn't do what it looks like it does.

## How to fix

Before:
```cpp
void f(char *p) {
    if (p + 12 == 0) {} // <- relies on pointer overflow, which is UB
}
```

After:
```cpp
void f(char *p) {
    if (p == nullptr) {}
}
```

## Related checkers

- [invalidTestForOverflow.md](invalidTestForOverflow.md) - a related undefined-behaviour trap, relying
  on signed integer overflow instead of pointer overflow.
