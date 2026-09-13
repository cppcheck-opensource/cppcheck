# invalidContainerLoop

**Message**: Calling 'push_back' while iterating the container is invalid.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C++

## Description

A container is modified with a call that can invalidate iterators (such as `push_back()` on a
`vector`) while a loop is iterating over that same container.

## Motivation

Modifying a container while a range-based (or iterator-based) loop is walking over it risks invalidating
the loop's own iterator mid-iteration - for `push_back()` on a `vector`, this happens whenever the call
needs to reallocate storage, which isn't certain from any single call but is guaranteed to happen
eventually as the container grows. Once that happens, the loop machinery's next step (comparing,
dereferencing, or incrementing the now-invalid iterator) is undefined behaviour - even though it often
appears to "work" for a while before the container's storage actually needs to move.

## How to fix

Before:
```cpp
#include <vector>
void f(std::vector<int> v) {
    for (auto i : v) {
        if (i < 5)
            v.push_back(i * 2); // <- invalidContainerLoop: modifying 'v' while iterating over it
    }
}
```

After:
```cpp
#include <vector>
void f(std::vector<int> v) {
    std::vector<int> toAdd;
    for (auto i : v) {
        if (i < 5)
            toAdd.push_back(i * 2);
    }
    v.insert(v.end(), toAdd.begin(), toAdd.end());
}
```

## Related checkers

- [invalidContainer.md](invalidContainer.md) - the more general check for using a pointer, reference,
  or iterator into a container after some other call may have invalidated it (not necessarily while
  iterating).
