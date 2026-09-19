# incorrectStringCompare

**Message**: String literal "Hello" doesn't match length argument for substr().<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

`x.substr(pos, len) == "literal"` where `len` doesn't match the length of `"literal"`.

## Motivation

`substr(pos, len)` only ever returns a string of (at most) `len` characters, so comparing it against a
literal of a different length can never be true (if `len` is too short) or is comparing more characters
than were actually extracted (if `len` is too long) - either way, the comparison doesn't test what it
looks like it tests.

## How to fix

Before:
```cpp
#include <string>
int f(std::string test) {
    return test.substr(0, 4) == "Hello" ? 0 : 1; // <- "Hello" is 5 chars
}
```

After:
```cpp
#include <string>
int f(std::string test) {
    return test.substr(0, 5) == "Hello" ? 0 : 1;
}
```
