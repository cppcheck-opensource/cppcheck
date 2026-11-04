# knownEmptyContainer

**Message**: Iterating over container 'v' that is always empty.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C++

## Description

A container or iterator that cppcheck knows is always empty is iterated over or otherwise used as if
it might contain elements.

## Motivation

Code that iterates over a container cppcheck can prove is empty at that point is dead code - the loop
body never runs, which is usually not what the author intended.

## How to fix

Before:
```cpp
#include <vector>
void f() {
    std::vector<int> v;
    for (auto x : v) {} // <- knownEmptyContainer: 'v' is provably empty here
}
```

After:
```cpp
#include <vector>
void f(std::vector<int> v) {
    for (auto x : v) {}
}
```
