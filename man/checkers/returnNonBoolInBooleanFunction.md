# returnNonBoolInBooleanFunction

**Message**: Non-boolean value returned from function returning bool<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C++

## Description

A function declared to return `bool` returns a value that cppcheck can determine is not `0` or `1`.

## Motivation

A `bool`-returning function is expected to always produce `true`/`false`; returning a value outside
that range (after the implicit conversion, anything nonzero collapses to `true` anyway) usually signals
that the wrong expression was returned, or that the function's intent was actually to return a count or
status code.

## How to fix

Before:
```cpp
bool f() {
    return 2; // <- collapses to 'true', likely not intended
}
```

After:
```cpp
bool f() {
    return true;
}
```
