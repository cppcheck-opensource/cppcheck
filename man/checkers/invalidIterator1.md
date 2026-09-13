# invalidIterator1

**Message**: Invalid iterator: aIt<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C++

## Description

An iterator that hasn't been given a value yet (or is no longer valid) is passed to
`insert()`/`erase()`.

## Motivation

Passing an iterator that doesn't currently point anywhere valid into a container method is undefined
behaviour, and is easy to miss when the same iterator was previously used (and invalidated) earlier in
the function.

## How to fix

Before:
```cpp
#include <list>
void f(const std::list<int>& m) {
    std::list<int>::iterator aIt = m.begin();
    m.erase(*aIt);
    m.erase(aIt); // <- invalidIterator1: 'aIt' was already erased by value above
}
```

After:
```cpp
#include <list>
void f(std::list<int>& m) {
    std::list<int>::iterator aIt = m.begin();
    m.erase(aIt);
}
```

## Related checkers

- [eraseDereference.md](eraseDereference.md) - a related mistake where an iterator is dereferenced,
  rather than passed to `erase()`, after the element it pointed to was already erased.
