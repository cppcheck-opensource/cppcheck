# iterateByValue

**Message**: Variable 'x' is used to iterate by value. It could be declared as a const reference which is usually faster and recommended in C++.<br/>
**Category**: Code Quality<br/>
**Severity**: Performance<br/>
**Language**: C++

## Description

A range-based `for` loop's variable (`for (auto x : container)`) copies each element - copying is
wasteful when a `const` reference would do.

## Motivation

Copying every element of a container just to read it wastes time and memory proportional to the
element's size and the container's length, for no benefit over a `const` reference.

## How to fix

Before:
```cpp
#include <set>
#include <string>
void f() {
    const std::set<std::string> ss = { "a", "b", "c" };
    for (auto s : ss) // <- each string is copied
        (void)s.size();
}
```

After:
```cpp
#include <set>
#include <string>
void f() {
    const std::set<std::string> ss = { "a", "b", "c" };
    for (const auto& s : ss)
        (void)s.size();
}
```

## Related checkers

- [passedByValue.md](passedByValue.md) - the same idea, for a function parameter instead of a loop
  variable.
- [iterateByValueCallback.md](iterateByValueCallback.md) - the same idea, when the loop is inside a
  callback function.
