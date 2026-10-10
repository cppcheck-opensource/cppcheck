# ignoredReturnValue and ignoredReturnErrorCode

**Message**: Return value of function $symbol() is not used.<br/>
**Category**: Correctness<br/>
**Severity**: Warning/Style<br/>
**Language**: C/C++

## Description

The return value of a function that must be checked is discarded:

- `ignoredReturnValue`: the function is marked `[[nodiscard]]`, or is known to be pure/const, or
  returns newly allocated memory that would otherwise leak.
- `ignoredReturnErrorCode`: the function's return value follows a "returns an error code" convention
  (per its library configuration), so discarding it means a failure could go unnoticed.

## Motivation

If a function's whole purpose is its return value (a pure calculation), or its return value is the only
way to learn it failed, throwing that value away either wastes the call entirely or hides a possible
failure that will only surface later, in a more confusing way.

## How to fix

Before:
```cpp
#include <cstring>
void f(char* a, char* b) {
    strcmp(a, b); // <- the comparison result is thrown away
}
```

After:
```cpp
#include <cstring>
void f(char* a, char* b) {
    if (strcmp(a, b) == 0) {}
}
```
