# redundantInitialization

**Message**: Redundant initialization for 'x'. The initialized value is overwritten before it is read.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

A variable's initializer value is overwritten before it's read - the same idea as a redundant
assignment, but for the value given at declaration.

## Motivation

Giving a variable an initial value only for it to be immediately replaced wastes the work of computing
that initial value, and can mislead a reader into thinking the initializer matters.

## How to fix

Before:
```cpp
#include <string>
std::string f() {
    std::string s = "abc";
    s = "def"; // <- the "abc" initializer is never read
    return s;
}
```

After:
```cpp
#include <string>
std::string f() {
    std::string s = "def";
    return s;
}
```

## Related checkers

- [redundantAssignment.md](redundantAssignment.md) - the same idea for a later assignment instead of the initializer.
