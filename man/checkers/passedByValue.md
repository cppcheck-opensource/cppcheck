# passedByValue

**Message**: Parameter 'x' should be passed by const reference.<br/>
**Category**: Code Quality<br/>
**Severity**: Performance<br/>
**Language**: C++

## Description

A function parameter of a class/struct/container type (not a small built-in type) is passed by value
and never modified, so it's copied for nothing - it could be a `const` reference instead.

## Motivation

Passing a non-trivial object by value makes an unnecessary copy every time the function is called; a
`const` reference avoids the copy while still preventing the function from modifying the caller's
object.

## How to fix

Before:
```cpp
#include <string>
#include <cstdio>
void f(std::string str) { // <- an unnecessary copy of 'str' is made
    printf("%s\n", str.c_str());
}
```

After:
```cpp
#include <string>
#include <cstdio>
void f(const std::string& str) {
    printf("%s\n", str.c_str());
}
```

## Related checkers

- [passedByValueCallback.md](passedByValueCallback.md) - the same idea, but for a parameter of a
  function used as a callback.
- [iterateByValue.md](iterateByValue.md) - the same idea, for a range-based `for` loop's variable.
