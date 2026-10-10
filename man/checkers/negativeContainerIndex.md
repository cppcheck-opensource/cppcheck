# negativeContainerIndex

**Message**: Array index -11 is out of bounds.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error/Warning<br/>
**Language**: C++

## Description

A container is indexed with a value known to be negative.

## Motivation

Indexing an array-like container with a negative value is undefined behaviour - there is no valid
element "before the beginning" to access.

## How to fix

Before:
```cpp
#include <vector>
void f(const std::vector<int> &v) {
    v[-11] = 123; // <- negativeContainerIndex
}
```

After:
```cpp
#include <vector>
void f(const std::vector<int> &v) {
    v[11] = 123;
}
```

## Related checkers

- [containerOutOfBounds.md](containerOutOfBounds.md) - the more general out-of-bounds container access
  check, for indices that are too large rather than negative.
