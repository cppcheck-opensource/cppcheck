# mismatchingContainers

**Message**: Iterators of different containers 'l1' and 'l2' are used together.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C++

## Description

An iterator that belongs to one container is compared (with `!=`/`==`, or via a relational operator)
against an iterator from a different container - most often seen as the loop condition of a `for` loop
that begins with one container's iterator and ends with another's.

## Motivation

Comparing iterators from two different containers is meaningless - there's no relationship between
their positions - and is almost always a copy-paste mistake, typically a loop's end condition that
still refers to the wrong container.

## How to fix

Before:
```cpp
#include <list>
void f() {
    std::list<int> l1;
    std::list<int> l2;
    for (std::list<int>::iterator it = l1.begin(); it != l2.end(); ++it) { } // <- mismatchingContainers
}
```

After:
```cpp
#include <list>
void f() {
    std::list<int> l1;
    for (std::list<int>::iterator it = l1.begin(); it != l1.end(); ++it) { }
}
```

## Related checkers

- [iterators1.md](iterators1.md) / [mismatchingContainerIterator.md](mismatchingContainerIterator.md) -
  the equivalent mistake when the mismatched iterator is passed to a container method instead of
  compared.
- [mismatchingContainerExpression.md](mismatchingContainerExpression.md) - a related, weaker check for
  when cppcheck can't prove two iterator-producing expressions refer to the same container at all.
