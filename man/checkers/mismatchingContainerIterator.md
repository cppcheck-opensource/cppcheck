# mismatchingContainerIterator

**Message**: Iterator 'it' referring to container 'l1' is used with container 'l2'.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C++

## Description

An iterator that belongs to one container is passed to an `insert()`/`erase()`-style method of a
different container.

## Motivation

An iterator only makes sense in the context of the container it came from. Passing it to a different
container's method is meaningless and undefined behaviour - almost always a copy-paste mistake where
the wrong container variable was used.

## How to fix

Before:
```cpp
#include <list>
void foo() {
    std::list<int> l1;
    std::list<int> l2;
    std::list<int>::iterator it = l1.begin();
    l2.insert(it, 0); // <- mismatchingContainerIterator: 'it' belongs to l1, not l2
}
```

After:
```cpp
#include <list>
void foo() {
    std::list<int> l1;
    std::list<int>::iterator it = l1.begin();
    l1.insert(it, 0);
}
```

## Related checkers

- [iterators1.md](iterators1.md) - the same code shape is also caught by this related, differently
  worded check; both messages can appear together on the same line.
- [mismatchingContainers.md](mismatchingContainers.md) - the equivalent mistake when the mismatched
  iterators are compared (`!=`/`==`) rather than passed to a container method.
