# uselessCallsSubstr

**Message**: Ineffective call of function 'substr' because it returns a copy of the object. Use operator= instead.<br/>
**Category**: Performance<br/>
**Severity**: Performance<br/>
**Language**: C++

## Description

A `substr()` call whose arguments make it a no-op (returns an unmodified copy of the whole string),
always return an empty string, or (when assigned back to the same string) just a slower way to write
`resize()`/`pop_back()`/`replace()`.

## Motivation

`substr()` copies characters into a new string - when the requested range happens to be the whole
string, an empty range, or a prefix/suffix of the string it's being assigned back into, that copy is
pure overhead compared to the more direct operation (`operator=`, `resize()`, `pop_back()`, `replace()`)
that achieves the same result.

## How to fix

Before:
```cpp
#include <string>
void f() {
    std::string s1, s2;
    s2 = s1.substr(); // <- substr() with no arguments just copies the whole string
}
```

After:
```cpp
#include <string>
void f() {
    std::string s1, s2;
    s2 = s1;
}
```
