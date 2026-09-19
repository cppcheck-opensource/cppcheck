# containerOutOfBoundsIndexExpression

**Message**: Out of bounds access of s, index 's.size()' is out of bounds.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C++

## Description

An index expression itself (for example `s[s.size()]`) provably reaches or exceeds the container's own
size.

## Motivation

Accessing a container out of bounds is undefined behaviour. This particular shape is easy to write by
accident - `s.size()` looks like it should be the last valid position, but it is actually one past it.

## How to fix

Before:
```cpp
#include <string>
void f(std::string s) {
    s[s.size()] = 1; // <- containerOutOfBoundsIndexExpression: one past the last character
}
```

After:
```cpp
#include <string>
void f(std::string s) {
    s[s.size() - 1] = 1;
}
```

## Related checkers

- [containerOutOfBounds.md](containerOutOfBounds.md) - the more general out-of-bounds container access
  check that this is a special case of.
