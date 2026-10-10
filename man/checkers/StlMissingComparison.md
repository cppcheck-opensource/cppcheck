# StlMissingComparison

**Message**: Missing bounds check for extra iterator increment in loop.<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C++

## Description

Inside a loop, the iterator is incremented a second time (in addition to the loop's own increment)
without any bounds check in between, risking incrementing it past `end()`.

## Motivation

An iterator that's advanced twice per iteration without checking for `end()` in between can walk right
past the end of the container - dereferencing it afterwards, or even just comparing it again, is then
undefined behaviour. This check flags the missing safety check itself, not a proven out-of-bounds
increment - if the container always happens to have enough elements left when the extra increment runs,
the loop never actually goes past `end()` in practice, even though the check that would guarantee this is
absent.

## How to fix

Before:
```cpp
#include <set>
void f(std::set<int> &ints, bool a) {
    for (std::set<int>::iterator it = ints.begin(); it != ints.end(); ++it) {
        if (a) {
            it++; // <- StlMissingComparison: might increment 'it' past end()
        }
    }
}
```

After: don't increment the iterator a second time inside the loop body.
```cpp
#include <set>
void f(std::set<int> &ints, bool a) {
    for (std::set<int>::iterator it = ints.begin(); it != ints.end(); ++it) {
        if (a) {
        }
    }
}
```
