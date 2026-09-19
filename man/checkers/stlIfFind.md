# stlIfFind and stlIfStrFind

**Message**: Suspicious condition. The result of find() is an iterator, but it is not properly checked.<br/>
**Category**: Correctness<br/>
**Severity**: Warning/Performance<br/>
**Language**: C++

## Description

The result of a container's `.find()` is used directly as a boolean condition:

- `stlIfFind`: this tests whether the resulting iterator happens to be "truthy", not whether the
  element was found, which is virtually never what's intended.
- `stlIfStrFind`: the same mistake specifically for `std::string::find()`, which returns a *position*,
  not an iterator - comparing it directly as a boolean is wrong far more often than not (position 0 -
  a match at the very start of the string - is falsy). With a C++20-or-later standard, this is instead
  reported as a performance suggestion to use `string::starts_with()`.

## Motivation

`find()` on most containers returns an iterator (or, for `std::string`, a position) that must be
compared against `.end()` (or `std::string::npos`) to know whether something was actually found. Using
the raw result as a boolean condition compiles cleanly and can even look like it works in quick testing,
but it is testing the wrong thing - a genuine logic bug hiding behind code that reads as correct.

## How to fix

Before:
```cpp
#include <set>
void f(std::set<int> s) {
    if (s.find(12)) { } // <- stlIfFind: this checks the iterator's "truthiness", not whether 12 was found
}
```

After:
```cpp
#include <set>
void f(std::set<int> s) {
    if (s.find(12) != s.end()) { }
}
```

Before:
```cpp
#include <string>
void f(const std::string &s) {
    if (s.find("abc")) { } // <- stlIfStrFind: position 0 (a match at the start) is falsy here
}
```

After:
```cpp
#include <string>
void f(const std::string &s) {
    if (s.find("abc") != std::string::npos) { }
}
```

## Related checkers

- [stlFindInsert.md](stlFindInsert.md) - a different `find()`-related redundancy, checking before
  inserting into an associative container.
