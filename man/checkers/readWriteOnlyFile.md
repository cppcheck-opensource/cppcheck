# readWriteOnlyFile

**Message**: Read operation on a file that was opened only for writing.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C/C++

## Description

The file was opened in a mode (`"w"`) that only allows writing, and the code then reads from it.

## Motivation

Reading from a stream that was opened write-only is undefined behaviour in the C standard - even where
an implementation happens to do something predictable with it, code relying on that isn't portable.
cppcheck follows a `FILE*`'s open mode only through straight-line code in a single function; once it's
passed to another function, or is a global/member variable, the tracking is dropped rather than
guessed at, so this check finding nothing does not mean a file is necessarily being used consistently
with how it was opened.

## How to fix

Before:
```cpp
#include <cstdio>
void f() {
    FILE *fp = fopen("a.txt", "w");
    if (!fp) return;
    char buf[10];
    fread(buf, 1, 10, fp); // <- 'fp' was only opened for writing
    fclose(fp);
}
```

After:
```cpp
#include <cstdio>
void f() {
    FILE *fp = fopen("a.txt", "r");
    if (!fp) return;
    char buf[10];
    fread(buf, 1, 10, fp);
    fclose(fp);
}
```

## Related checkers

- [writeReadOnlyFile.md](writeReadOnlyFile.md) - the opposite mismatch: writing to a read-only file.
- [useClosedFile.md](useClosedFile.md), [IOWithoutPositioning.md](IOWithoutPositioning.md),
  [seekOnAppendedFile.md](seekOnAppendedFile.md), [incompatibleFileOpen.md](incompatibleFileOpen.md) -
  other checks that follow the same `FILE*` through a function.
