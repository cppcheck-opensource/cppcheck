# stlFindInsert

**Message**: Searching before insertion is not necessary.<br/>
**Category**: Performance<br/>
**Severity**: Performance<br/>
**Language**: C++

## Description

An associative container is searched with `.find()`, and if nothing was found, the same key is
inserted right afterwards - the search is redundant since `insert()`/`emplace()` already report whether
the key existed.

## Motivation

Searching a container and then inserting into it based on the result does the lookup twice: once
explicitly with `.find()`, and again internally inside `insert()`/`emplace()` to find the right place
(or detect the key is already present). Going straight to `insert()`/`emplace()` gets the same behavior
in one lookup instead of two.

## How to fix

Before:
```cpp
#include <set>
void f1(std::set<unsigned>& s, unsigned x) {
    if (s.find(x) == s.end()) { // <- redundant, insert() already knows if x is present
        s.insert(x);
    }
}
```

After:
```cpp
#include <set>
void f1(std::set<unsigned>& s, unsigned x) {
    s.insert(x);
}
```

## Related checkers

- [redundantIfRemove.md](redundantIfRemove.md) - a similar redundant-check-before-a-safe-operation
  pattern, for `remove()` instead of `insert()`.
- [stlIfFind.md](stlIfFind.md) - a different `find()`-related mistake, where the result is misread as a
  boolean.
