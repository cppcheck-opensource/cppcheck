# stlOutOfBounds

**Message**: When ii==foo.size(), foo.at(ii) is out of bounds.<br/>
**Category**: Correctness<br/>
**Severity**: Error<br/>
**Language**: C++

## Description

A loop of the form `for (i = 0; i <= container.size(); ++i)` uses `<=` instead of `<`, so the last
iteration indexes one past the end.

## Motivation

This is one of the most common off-by-one mistakes in loops that walk a container by index: the loop
condition looks like a natural "up to and including the size" check, but the valid indices only go up
to `size() - 1`.

## How to fix

Before:
```cpp
#include <vector>
void f(std::vector<int> foo) {
    for (unsigned int ii = 0; ii <= foo.size(); ++ii) { // <- stlOutOfBounds
       foo.at(ii) = 0;
    }
}
```

After:
```cpp
#include <vector>
void f(std::vector<int> foo) {
    for (unsigned int ii = 0; ii < foo.size(); ++ii) {
       foo.at(ii) = 0;
    }
}
```

## Related checkers

- [containerOutOfBounds.md](containerOutOfBounds.md) - the more general out-of-bounds container access
  check that this loop-condition mistake is a specific cause of.
