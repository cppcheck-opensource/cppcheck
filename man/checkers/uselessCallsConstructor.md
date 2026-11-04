# uselessCallsConstructor

**Message**: Inefficient constructor call: container 'x' is assigned a partial copy of itself. Use erase() or resize() instead.<br/>
**Category**: Performance<br/>
**Severity**: Performance<br/>
**Language**: C++

## Description

A container is assigned a range constructed from its own begin/(begin+offset) - this makes an
unnecessary temporary copy of part of the container's own data, when `erase()`/`resize()` would do the
same trim in place.

## Motivation

Building a whole new range-constructed container from a slice of a container's own elements, just to
assign it back over the original, copies data that's already sitting in the right place - an in-place
trim (`erase()`/`resize()`) achieves the same final content without the temporary copy.

## How to fix

Before:
```cpp
#include <string>
std::string f(std::string s, std::size_t end) {
    s = { s.begin(), s.begin() + end }; // <- an unnecessary partial self-copy
    return s;
}
```

After:
```cpp
#include <string>
std::string f(std::string s, std::size_t end) {
    s.resize(end);
    return s;
}
```
