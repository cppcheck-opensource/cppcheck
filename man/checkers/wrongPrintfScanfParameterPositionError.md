# wrongPrintfScanfParameterPositionError

**Message**: printf: referencing parameter 4 while 3 arguments given.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

A POSIX positional specifier (`%2$d`) references an argument number that doesn't exist, or numbering
starts at `0` instead of `1`.

## Motivation

POSIX positional format specifiers let a format string reference its arguments out of order, but that
also means a typo in the position number silently reads the wrong argument, or - as checked here - one
that isn't even there.

## How to fix

Before:
```cpp
#include <cstdio>
void foo() {
    printf("%1$d, %d, %4$d\n", 1, 2, 3); // <- no 4th argument
}
```

After:
```cpp
#include <cstdio>
void foo() {
    printf("%1$d, %d, %3$d\n", 1, 2, 3);
}
```

## Related checkers

- [wrongPrintfScanfArgNum.md](wrongPrintfScanfArgNum.md) - the plain (non-positional) argument-count mismatch.
