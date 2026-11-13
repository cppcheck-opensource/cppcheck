# mismatchingContainerExpression

**Message**: Iterators to containers from different expressions 'f()' and 'g()' are used together.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Warning<br/>
**Language**: C++

## Description

Two iterators come from two separate calls to same-looking expressions (for example `begin(f())` and
`end(g())`) that cppcheck can't prove refer to the same container.

## Motivation

When the container itself is the result of a function call rather than a plain variable, it's easy to
accidentally call two different functions (or the same function twice, if it returns a different
container each time) for what was meant to be a single container's begin/end pair.

## How to fix

Before:
```cpp
#include <vector>
#include <algorithm>
std::vector<int>& f();
std::vector<int>& g();
void foo() {
    (void)std::find(begin(f()), end(g()), 0); // <- mismatchingContainerExpression
}
```

After:
```cpp
#include <vector>
#include <algorithm>
std::vector<int>& f();
void foo() {
    (void)std::find(begin(f()), end(f()), 0);
}
```

## Related checkers

- [mismatchingContainers.md](mismatchingContainers.md) - the stronger version of this check, for when
  cppcheck can tell the two iterators definitely belong to different containers (rather than merely
  being unable to prove they're the same one).
