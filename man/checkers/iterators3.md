# iterators3

**Message**: Same iterator is used with containers 'l1' that are temporaries or defined in different scopes.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C++

## Description

An iterator is compared against, or used with, a container of the same name that's actually a
different variable - a temporary, or one declared in a different, unrelated scope that happens to
shadow the container the iterator really belongs to.

## Motivation

Two variables sharing a name in different scopes are still two entirely different objects. An iterator
taken from one of them has no valid relationship to the other, even though the code reads as if it does
because the name is identical - comparing or using it against the wrong one is undefined behaviour.

## How to fix

Before:
```cpp
#include <vector>
std::vector<int> f();
bool foo() {
    return f().begin() != f().end(); // <- iterators3: each call to f() returns a different temporary
}
```

After: call the function once and compare against the same instance.
```cpp
#include <vector>
std::vector<int> f();
bool foo() {
    std::vector<int> v = f();
    return v.begin() != v.end();
}
```

## Related checkers

- [iterators1.md](iterators1.md) - the same idea, but for two containers that are genuinely
  different variables (not just same-named ones in different scopes).
- [mismatchingContainerIterator.md](mismatchingContainerIterator.md) - the same idea, for passing the
  iterator into another container's method instead of comparing it.
