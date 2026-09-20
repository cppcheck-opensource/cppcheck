# seekOnAppendedFile

**Message**: Repositioning operation performed on a file opened in append mode has no effect.<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

A repositioning call (`fseek()`, `fsetpos()`, `rewind()`) is made on a file that was opened in append
mode (`"a"`) - every write in append mode always goes to the end of the file regardless, so
repositioning has no effect.

## Motivation

Code that repositions before writing to an append-mode file is misleading: the reposition is silently
ignored, so the code doesn't do what it looks like it does, and the call is dead weight.

## How to fix

Before:
```cpp
#include <cstdio>
void f() {
    FILE *fp = fopen("a.txt", "a");
    if (!fp) return;
    fseek(fp, 0, SEEK_SET); // <- has no effect in append mode
    fwrite("x", 1, 1, fp);
    fclose(fp);
}
```

After:
```cpp
#include <cstdio>
void f() {
    FILE *fp = fopen("a.txt", "a");
    if (!fp) return;
    fwrite("x", 1, 1, fp);
    fclose(fp);
}
```

## Related checkers

- [useClosedFile.md](useClosedFile.md), [readWriteOnlyFile.md](readWriteOnlyFile.md),
  [writeReadOnlyFile.md](writeReadOnlyFile.md), [IOWithoutPositioning.md](IOWithoutPositioning.md),
  [incompatibleFileOpen.md](incompatibleFileOpen.md) - other checks that follow the same `FILE*` through
  a function.
