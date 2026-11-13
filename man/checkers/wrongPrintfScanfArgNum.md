# wrongPrintfScanfArgNum

**Message**: printf format string requires 2 parameters but only 1 is given.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error/Warning<br/>
**Language**: C/C++

## Description

The format string's conversion specifiers (`%d`, `%s`, ...) don't match the number of arguments
actually given to a `printf`/`scanf`-family call - too many or too few.

## Motivation

`printf`/`scanf` format strings are not type- or count-checked by the C or C++ language itself - the
compiler trusts that whatever conversion specifiers you wrote match whatever arguments follow. Too few
arguments means a conversion reads garbage memory as if it were a real argument; too many just means
wasted arguments, but often signals a forgotten `%` specifier.

## How to fix

Before:
```cpp
#include <cstdio>
void f() {
    printf("%d%s", 1); // <- format string needs 2 arguments, only 1 given
}
```

After:
```cpp
#include <cstdio>
void f() {
    printf("%d%s", 1, "x");
}
```

## Related checkers

- [wrongPrintfScanfParameterPositionError.md](wrongPrintfScanfParameterPositionError.md) - a related mistake with POSIX positional specifiers (`%2$d`).
