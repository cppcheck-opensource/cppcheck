# writeReadOnlyFile

**Message**: Write operation on a file that was opened only for reading.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C/C++

## Description

The file was opened in a mode (`"r"`) that only allows reading, and the code then writes to it.

## Motivation

Writing to a stream that was opened read-only is undefined behaviour in the C standard - even where an
implementation happens to do something predictable with it, code relying on that isn't portable.

## How to fix

Before:
```cpp
#include <cstdio>
void f() {
    FILE *fp = fopen("a.txt", "r");
    if (!fp) return;
    fwrite("x", 1, 1, fp); // <- 'fp' was only opened for reading
    fclose(fp);
}
```

After:
```cpp
#include <cstdio>
void f() {
    FILE *fp = fopen("a.txt", "w");
    if (!fp) return;
    fwrite("x", 1, 1, fp);
    fclose(fp);
}
```

## Related checkers

- [readWriteOnlyFile.md](readWriteOnlyFile.md) - the opposite mismatch: reading from a write-only file.
- [useClosedFile.md](useClosedFile.md), [IOWithoutPositioning.md](IOWithoutPositioning.md),
  [seekOnAppendedFile.md](seekOnAppendedFile.md), [incompatibleFileOpen.md](incompatibleFileOpen.md) -
  other checks that follow the same `FILE*` through a function.
