# rethrowNoCurrentException

**Message**: Rethrowing current exception with 'throw;', it seems there is no current exception to rethrow. If there is no current exception this calls std::terminate().<br/>
**Category**: Correctness<br/>
**Severity**: Error<br/>
**Language**: C++ only

## Description

A bare `throw;` appears somewhere that isn't inside a `catch` block. If there's no exception currently
being handled, this calls `std::terminate()`.

## Motivation

A bare `throw;` only makes sense while an exception is actively being handled (inside a `catch` block,
or a function called from one, with the exception still propagating) - used anywhere else, there's no
exception to rethrow, and the C++ standard says the result is a call to `std::terminate()`, aborting the
program.

## How to fix

Before:
```cpp
void f() {
    throw; // <- not inside a catch block
}
```

After:
```cpp
void doWork();
void f() {
    try {
        doWork();
    } catch (...) {
        throw; // fine here: there is a current exception being handled
    }
}
```
