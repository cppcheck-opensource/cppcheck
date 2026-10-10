# redundantCopyLocalConst

**Message**: Use const reference for 'x' to avoid unnecessary data copying.<br/>
**Category**: Performance<br/>
**Severity**: Performance<br/>
**Language**: C/C++

## Description

A local variable is declared `const` and initialized as a copy of an existing object that outlives it -
since the copy is never modified, a `const` reference would do the same job without copying.

## Motivation

Copying an object (a `std::string`, a container, any non-trivial type) costs time and memory for no
benefit when the copy is only ever read. A `const&` avoids the copy entirely while behaving identically
from the reader's point of view.

## How to fix

Before:
```cpp
#include <string>
void f(std::string str) {
    std::string s2 = str; // <- 's2' is never modified
}
```

After:
```cpp
#include <string>
void f(const std::string& str) {
    const std::string& s2 = str;
}
```

## Related checkers

- [redundantCopy.md](redundantCopy.md) - the buffer-write equivalent of this idea.
