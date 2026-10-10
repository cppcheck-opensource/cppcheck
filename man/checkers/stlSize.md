# stlSize

**Message**: Possible inefficient checking for 'x' emptiness.<br/>
**Category**: Performance<br/>
**Severity**: Performance<br/>
**Language**: C++ (only checked with a pre-C++11 standard, and only for `std::list`)

## Description

`.size()` is compared against `0`/`1` on a container whose `.size()` isn't guaranteed to be
constant-time (only checked pre-C++11, and only for `std::list`) - `.empty()` is always at least as
fast.

## Motivation

Before C++11, `std::list::size()` was allowed to take time proportional to the number of elements
(implementations were free to compute it by walking the list), while `.empty()` is always O(1).
Comparing `.size()` against `0`/`1` where `.empty()` would do is therefore a real, if usually small,
performance cost, and easy to write out of habit from code that works with other containers.

## How to fix

Before:
```cpp
#include <list>
struct Fred {
    void foo();
    std::list<int> x;
};
void Fred::foo() {
    if (x.size() == 0) {} // <- only with a pre-C++11 standard: size() isn't guaranteed O(1)
}
```

After:
```cpp
#include <list>
struct Fred {
    void foo();
    std::list<int> x;
};
void Fred::foo() {
    if (x.empty()) {}
}
```
