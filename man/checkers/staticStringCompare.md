# staticStringCompare and stringCompare

**Message**: Unnecessary comparison of static strings.<br/>
**Category**: Code Quality<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

Two string literals (`staticStringCompare`), or two identical-looking string variables
(`stringCompare`), are compared with a string-comparison function (`strcmp`, `stricmp`, `wcscmp`, ...) -
the result is already known at compile time either way.

## Motivation

Comparing two string literals, or a variable against itself under a different name, can never vary at
runtime - the comparison's result is fixed regardless of any input, which usually means a variable name
was typed wrong, or the comparison is leftover from earlier code that used to compare something real.

## How to fix

Before:
```cpp
#include <cstring>
int main() {
    if (strcmp("00FF00", "00FF00") == 0) {} // <- always true
}
```

After:
```cpp
#include <cstring>
int main(const char* value) {
    if (strcmp(value, "00FF00") == 0) {} // compare the actual variable
}
```

## Related checkers

- [literalWithCharPtrCompare.md](literalWithCharPtrCompare.md) - a related string-comparison mistake:
  comparing a pointer against a literal with `==` instead of a string-comparison function.
