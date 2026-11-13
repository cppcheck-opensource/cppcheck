# eraseIteratorOutOfBounds and eraseIteratorOutOfBoundsCond

**Message**: Calling function 'erase()' on the iterator 'v.begin()' which is out of bounds.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error/Warning<br/>
**Language**: C++

## Description

`erase()` is called with an iterator that is known to be `end()`, before `begin()`, or otherwise out of
the container's range.

- `eraseIteratorOutOfBounds`: the iterator is unconditionally known to be out of bounds.
- `eraseIteratorOutOfBoundsCond`: the iterator is only out of bounds on one branch of a condition
  tested nearby - either that condition is redundant, or this `erase()` call is a bug.

## Motivation

Calling `erase()` with an iterator that doesn't point at an actual element in the container (`end()`,
or the result of moving an iterator before `begin()` or past `end()`) is undefined behaviour.

## How to fix

Before:
```cpp
#include <vector>
void f() {
    std::vector<int> v;
    v.erase(v.begin()); // <- eraseIteratorOutOfBounds: 'v' is empty, begin() == end()
}
```

After:
```cpp
#include <vector>
void f() {
    std::vector<int> v = {1, 2, 3};
    v.erase(v.begin());
}
```

Before:
```cpp
#include <vector>
void f(std::vector<int>& v, std::vector<int>::iterator it) {
    if (it == v.end()) {}
    v.erase(it); // <- eraseIteratorOutOfBoundsCond: 'it' can be end() here
}
```

After:
```cpp
#include <vector>
void f(std::vector<int>& v, std::vector<int>::iterator it) {
    if (it == v.end())
        return;
    v.erase(it);
}
```

## Related checkers

- [derefInvalidIterator.md](derefInvalidIterator.md) - the equivalent check for dereferencing (rather
  than erasing through) an iterator that could be out of bounds.
