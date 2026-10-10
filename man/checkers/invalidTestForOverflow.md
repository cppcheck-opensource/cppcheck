# invalidTestForOverflow

**Message**: Invalid test for overflow 'x+100&lt;x'; signed integer overflow is undefined behavior.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

A classic overflow check (`x + c < x`) that is itself undefined behaviour for signed integers and
pointers - optimizing compilers are allowed to, and do, remove such checks.

## Motivation

Signed integer overflow is undefined behaviour in C/C++, so a compiler is allowed to assume it never
happens - which means it can optimize away a check whose only purpose is to detect that overflow just
occurred. Code that relies on this pattern can pass in a debug build and silently stop working once
optimizations are turned on.

## How to fix

Before:
```cpp
void f(int x) {
    if (x + 100 < x) {} // <- relies on signed overflow, which is UB
}
```

After:
```cpp
#include <limits>
void f(int x) {
    if (x > std::numeric_limits<int>::max() - 100) {}
}
```

## Related checkers

- [pointerAdditionResultNotNull.md](pointerAdditionResultNotNull.md) - a related undefined-behaviour
  trap, relying on pointer overflow instead of signed integer overflow.
