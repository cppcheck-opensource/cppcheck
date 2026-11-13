# sameIteratorExpression

**Message**: Same iterators expression are used for algorithm.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C++

## Description

The exact same iterator expression is passed as both the "begin" and "end" argument of an algorithm,
making the range empty.

## Motivation

A range whose begin and end are the same iterator is always empty, so the algorithm call does nothing -
this is normally a sign that one of the two arguments was meant to be different (for example the end of
the container, not its beginning again).

## How to fix

Before:
```cpp
#include <algorithm>
void f(int a[10]) {
  if (std::any_of(&a[0], &a[0], [](int x){return x > 0;})) {} // <- sameIteratorExpression: an empty range
}
```

After:
```cpp
#include <algorithm>
void f(int a[10]) {
  if (std::any_of(&a[0], &a[10], [](int x){return x > 0;})) {}
}
```

## Related checkers

- [mismatchingContainers.md](mismatchingContainers.md) - the opposite mistake: two iterators that
  belong to different containers being used together.
