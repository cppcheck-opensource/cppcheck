# containerOutOfBounds

**Message**: Out of bounds access in 'v\[100\]', if 'v' size is 3 and '100' is 100<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error/Warning<br/>
**Language**: C++

## Description

An index or `.at()` call on a container is out of bounds for what cppcheck knows about the container's
size.

## Motivation

Accessing a container out of bounds is undefined behaviour - the same class of bug as an out-of-bounds
array access, but easier to introduce by accident because the container's internal storage can move or
resize without any visible syntax change at the call site.

## How to fix

Before:
```cpp
#include <vector>
void f() {
    std::vector<int> v(3);
    v[100] = 1; // <- containerOutOfBounds
}
```

After:
```cpp
#include <vector>
void f() {
    std::vector<int> v(3);
    v[2] = 1;
}
```

## Related checkers

- [containerOutOfBoundsIndexExpression.md](containerOutOfBoundsIndexExpression.md) - the same idea, but
  specifically when the index expression itself provably reaches or exceeds the container's size.
- [stlOutOfBounds.md](stlOutOfBounds.md) - the same idea, but specifically for a loop condition using
  `<=` instead of `<`.
- [negativeContainerIndex.md](negativeContainerIndex.md) - the same idea, but for a negative index.
