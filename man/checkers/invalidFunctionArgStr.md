# invalidFunctionArgStr

**Message**: Invalid $symbol() argument nr 1. A NUL-terminated string is required.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C/C++

## Description

A `char`/`wchar_t` buffer that isn't null-terminated (for example, the address of a single character)
is passed where a function (per its library configuration) requires a null-terminated C-string.

## Motivation

Functions like `strlen()`/`strcmp()` scan forward until they find a `'\0'` byte - if the argument isn't
actually a null-terminated string, whether this reads past the end of whatever memory was passed depends
on what byte values happen to follow it. If a `'\0'` byte never turns up before the buffer's real end,
the scan reads out of bounds, which is undefined behaviour; there's no way to guarantee that in advance
from the code alone, which is exactly why passing a non-null-terminated buffer is dangerous even when a
particular run happens not to crash.

## How to fix

Before:
```cpp
#include <cstring>
size_t f(char x) {
    return strlen(&x); // <- 'x' is a single char, not a null-terminated string
}
```

After:
```cpp
#include <cstring>
size_t f() {
    char x[] = "a";
    return strlen(x);
}
```

## Related checkers

- [invalidFunctionArg.md](invalidFunctionArg.md) - the same idea, but for an argument whose numeric
  value must fall in a specific range.
- [invalidFunctionArgBool.md](invalidFunctionArgBool.md) - the same idea, but for an argument that
  needs a real boolean rather than a plain `0`/`1`.
