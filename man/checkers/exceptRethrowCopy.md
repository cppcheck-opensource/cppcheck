# exceptRethrowCopy

**Message**: Throwing a copy of the caught exception instead of rethrowing the original exception.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C++ only

## Description

A `catch` block rethrows the caught exception with `throw x;` instead of a bare `throw;`, which makes
an unnecessary copy (and can slice a derived exception down to its base type).

## Motivation

`throw x;` constructs a new exception object from `x`, sliced down to whatever type `x` was declared
as - if the actual thrown object was a more-derived type, that extra information is lost. A bare
`throw;` rethrows the original exception object exactly as it was, with no copy and no slicing.

## How to fix

Before:
```cpp
#include <stdexcept>
void doWork();
void f() {
    try {
        doWork();
    } catch (const std::exception& err) {
        throw err; // <- copies (and can slice) the exception
    }
}
```

After:
```cpp
#include <stdexcept>
void doWork();
void f() {
    try {
        doWork();
    } catch (const std::exception& err) {
        throw;
    }
}
```

## Related checkers

- [catchExceptionByValue.md](catchExceptionByValue.md) - a related `catch`-clause mistake: catching by
  value instead of by reference.
