# IOWithoutPositioning

**Message**: Read and write operations without a call to a positioning function (fseek, fsetpos or rewind) or fflush in between result in undefined behaviour.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C/C++

## Description

A read is immediately followed by a write (or vice versa) on a file opened for both, with no
`fseek()`/`fsetpos()`/`rewind()`/`fflush()` in between - the C standard says this is undefined
behaviour.

## Motivation

The C standard requires a positioning call (or an `fflush()`) between a read and a following write (or
vice versa) on the same read/write stream. Skipping it is undefined behaviour, even though many
implementations happen to do something predictable with it. cppcheck only follows a local `FILE*`
variable through straight-line code in the function that opened it; a global or member file handle, or
passing the handle to another function, is enough uncertainty that it stops checking rather than guess,
so this only catches the mismatches it can actually prove.

## How to fix

Before:
```cpp
#include <cstdio>
void f() {
    FILE *fp = fopen("a.txt", "r+");
    if (!fp) return;
    char buf[10];
    fread(buf, 1, 10, fp);
    fwrite(buf, 1, 10, fp); // <- no seek/rewind/fflush since the read above
    fclose(fp);
}
```

After:
```cpp
#include <cstdio>
void f() {
    FILE *fp = fopen("a.txt", "r+");
    if (!fp) return;
    char buf[10];
    fread(buf, 1, 10, fp);
    fseek(fp, 0, SEEK_CUR);
    fwrite(buf, 1, 10, fp);
    fclose(fp);
}
```

## Related checkers

- [useClosedFile.md](useClosedFile.md), [readWriteOnlyFile.md](readWriteOnlyFile.md),
  [writeReadOnlyFile.md](writeReadOnlyFile.md), [seekOnAppendedFile.md](seekOnAppendedFile.md),
  [incompatibleFileOpen.md](incompatibleFileOpen.md) - other checks that follow the same `FILE*` through
  a function.
