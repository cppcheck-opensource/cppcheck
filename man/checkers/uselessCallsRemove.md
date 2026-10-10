# uselessCallsRemove

**Message**: Return value of std::remove() ignored. Elements remain in container.<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C++

## Description

The return value of `std::remove()`/`std::remove_if()`/`std::unique()` is ignored - these algorithms
don't actually shrink the container, they only move the elements to keep to the front and return the
new logical end; without also calling the container's `erase()` with that return value, the "removed"
elements are still there.

## Motivation

`std::remove()`/`std::remove_if()`/`std::unique()` cannot resize a container themselves (they only see
a pair of iterators, not the container), so they can only rearrange elements and report where the "new
end" is. Ignoring that return value leaves the container at its original size, with the elements that
were supposed to be removed still physically present (just in an unspecified state) between the new
logical end and the true end - the classic "erase-remove idiom" exists precisely to complete this.

## How to fix

Before:
```cpp
#include <vector>
#include <algorithm>
void f(std::vector<int> a, int val) {
    std::remove(a.begin(), a.end(), val); // <- elements aren't actually removed
}
```

After:
```cpp
#include <vector>
#include <algorithm>
void f(std::vector<int> a, int val) {
    a.erase(std::remove(a.begin(), a.end(), val), a.end());
}
```
