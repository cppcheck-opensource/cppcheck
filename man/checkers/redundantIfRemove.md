# redundantIfRemove

**Message**: Redundant checking of STL container element existence before removing it.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C++

## Description

`container.find(x) != container.end()` (or `container.find(x) != container.npos`) is checked before
calling `container.remove(x)` - `remove()` on an absent element is a harmless no-op, so the check adds
nothing.

## Motivation

The existence check reads as if it's guarding against a problem, but `remove()` on an element that
isn't there simply does nothing - it's not an error condition that needs to be avoided. The check is
pure overhead: an extra lookup, and an extra branch for a reader to understand, for no behavioral
difference.

## How to fix

Before:
```cpp
#include <string>
void f(std::string haystack, std::string needle) {
    if (haystack.find(needle) != haystack.end()) // <- remove() is safe on a miss
        haystack.remove(needle);
}
```

After:
```cpp
#include <string>
void f(std::string haystack, std::string needle) {
    haystack.remove(needle);
}
```

## Related checkers

- [stlFindInsert.md](stlFindInsert.md) - the same kind of redundant-check-before-a-safe-operation
  pattern, for `insert()` instead of `remove()`.
