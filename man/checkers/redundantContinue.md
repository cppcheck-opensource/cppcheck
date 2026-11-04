# redundantContinue

**Message**: 'continue' is redundant since it is the last statement in a loop.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

A `continue;` is the last statement in a loop body - the loop was about to move to its next iteration
anyway, so it has no effect.

## Motivation

A `continue;` in this position doesn't change what the code does, but it can mislead a reader into
thinking it matters, or that it's guarding something that comes after it in the loop body (when nothing
does).

## How to fix

Before:
```cpp
#include <cstdio>
void f() {
    for (int i = 0; i < 10; ++i) {
        printf("i = %d\n", i);
        continue; // <- this is already the end of the loop body
    }
}
```

After:
```cpp
#include <cstdio>
void f() {
    for (int i = 0; i < 10; ++i) {
        printf("i = %d\n", i);
    }
}
```

## Related checkers

- [unreachableCode.md](unreachableCode.md) - code that can never run at all, rather than a no-op statement.
