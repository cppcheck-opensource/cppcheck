# stlBoundaries

**Message**: Dangerous comparison using operator< on iterator.<br/>
**Category**: Correctness<br/>
**Severity**: Error<br/>
**Language**: C++

## Description

An iterator that doesn't support ordering (for example a `std::list` or `std::set` iterator) is
compared with `<`/`>` instead of `!=`/`==` - the relative order of two such iterators isn't guaranteed
to mean anything.

## Motivation

Only random-access iterators (like a `std::vector` or `std::deque` iterator) have a well-defined
"less than" relationship. For other container types, the underlying storage isn't laid out in a way
that makes iterator order meaningful, so comparing with `<`/`>` is not portable and not reliable.

## How to fix

Before:
```cpp
#include <list>
bool foo(std::list<int>::iterator it1, std::list<int>::iterator it2) {
    return it1 < it2; // <- stlBoundaries: order of list iterators isn't guaranteed
}
```

After:
```cpp
#include <list>
bool foo(std::list<int>::iterator it1, std::list<int>::iterator it2) {
    return it1 != it2;
}
```
