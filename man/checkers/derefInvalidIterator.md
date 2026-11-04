# derefInvalidIterator and derefInvalidIteratorRedundantCheck

**Message**: Dereference of an invalid iterator: v.begin()-1<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error/Warning<br/>
**Language**: C++

## Description

An iterator is dereferenced while it could be `end()`, before `begin()`, or otherwise invalid.

- `derefInvalidIterator`: cppcheck can tell directly that the iterator may be invalid at the point it's
  dereferenced.
- `derefInvalidIteratorRedundantCheck`: there's a nearby validity check on the same iterator, but the
  dereference happens where that check doesn't actually apply (for example, after the `if` that checked
  it, rather than inside it) - either the check is redundant, or the dereference is a bug.

## Motivation

Dereferencing an iterator that doesn't currently point at a real element is undefined behaviour, and is
easy to get subtly wrong when a validity check exists in the code but is written on the wrong side of
the dereference.

## How to fix

Before:
```cpp
#include <vector>
void f() {
    std::vector<int> v{ 1, 2, 3 };
    v.erase(v.begin() - 1); // <- derefInvalidIterator: 'v.begin()-1' is out of bounds
}
```

After:
```cpp
#include <vector>
void f() {
    std::vector<int> v{ 1, 2, 3 };
    v.erase(v.begin());
}
```

Before:
```cpp
#include <vector>
#include <algorithm>
int f(std::vector<int> v, int i) {
    auto it = std::find(v.begin(), v.end(), i);
    if (it != v.end()) {}
    return *it; // <- derefInvalidIteratorRedundantCheck: dereferenced outside the 'if' that checked it
}
```

After:
```cpp
#include <vector>
#include <algorithm>
int f(std::vector<int> v, int i) {
    auto it = std::find(v.begin(), v.end(), i);
    if (it != v.end())
        return *it;
    return -1;
}
```

## Related checkers

- [eraseIteratorOutOfBounds.md](eraseIteratorOutOfBounds.md) - the equivalent check for calling
  `erase()` with (rather than dereferencing) an iterator that could be out of bounds.
