# catchExceptionByValue

**Message**: Exception should be caught by reference.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C++ only

## Description

A `catch` clause catches a class type by value instead of by (const) reference.

## Motivation

Catching by value makes a copy of the exception object (and, if the actual thrown type is more derived
than the `catch` clause's type, slices it down, losing information) every time an exception is caught -
catching by `const&` avoids the copy entirely and preserves the exception's real, most-derived type.

## How to fix

Before:
```cpp
void doWork();
void f() {
    try {
        doWork();
    } catch (std::exception err) { // <- caught by value
    }
}
```

After:
```cpp
void doWork();
void f() {
    try {
        doWork();
    } catch (const std::exception& err) {
    }
}
```

## Related checkers

- [exceptRethrowCopy.md](exceptRethrowCopy.md) - a related `catch`-clause mistake: rethrowing with
  `throw x;` instead of a bare `throw;`.
