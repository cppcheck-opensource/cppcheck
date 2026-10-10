# literalWithCharPtrCompare and charLiteralWithCharPtrCompare

**Message**: String literal compared with variable 'c'. Did you intend to use strcmp() instead?<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

A `char*`/`wchar_t*` variable is compared directly against a string or character literal with `==`/`!=`
- this compares the pointer itself (its address), not the string's contents.

## Motivation

`c == "x"` compiles fine in C/C++ but almost never does what it looks like: it asks whether `c` happens
to point at the exact same memory as the literal `"x"`, not whether the characters match. This is one of
the most common C/C++ beginner mistakes, and the compiler gives no warning of its own.

## How to fix

Before:
```cpp
bool foo(char* c) {
    return c == "x"; // <- compares addresses, not contents
}
```

After:
```cpp
#include <cstring>
bool foo(char* c) {
    return strcmp(c, "x") == 0;
}
```

## Related checkers

- [staticStringCompare.md](staticStringCompare.md) - a related string-comparison mistake: comparing two
  string literals (or two identical variables) with a string-comparison function.
