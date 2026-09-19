# useClosedFile

**Message**: Used file that is not opened.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C/C++

## Description

A read, write, or positioning call is made on a `FILE*` after it's already been closed.

## Motivation

Using a file handle after `fclose()` is undefined behaviour - the underlying resource is gone, so any
further operation on it can't be relied on to do anything sensible.

## How to fix

Before:
```cpp
#include <cstdio>
void f() {
    FILE *f1 = fopen("a.txt", "r");
    if (!f1) return;
    fclose(f1);
    char buf[1];
    fread(buf, 1, 1, f1); // <- 'f1' is already closed
}
```

After:
```cpp
#include <cstdio>
void f() {
    FILE *f1 = fopen("a.txt", "r");
    if (!f1) return;
    char buf[1];
    fread(buf, 1, 1, f1);
    fclose(f1);
}
```

## Related checkers

- [readWriteOnlyFile.md](readWriteOnlyFile.md), [writeReadOnlyFile.md](writeReadOnlyFile.md),
  [IOWithoutPositioning.md](IOWithoutPositioning.md), [seekOnAppendedFile.md](seekOnAppendedFile.md),
  [incompatibleFileOpen.md](incompatibleFileOpen.md) - other checks that follow the same `FILE*` through
  a function to catch a different kind of open-mode/state mismatch.
