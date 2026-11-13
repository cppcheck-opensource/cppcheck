# incompatibleFileOpen

**Message**: The file 'a.txt' is opened for read and write access at the same time on different streams<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

The same filename is opened for reading on one stream while another stream already has it open for
writing (or vice versa).

## Motivation

Two independent streams onto the same file, one reading and one writing, can observe an inconsistent
view of the file's contents depending on buffering and timing - a portability and correctness hazard
that's easy to introduce without noticing, since each stream on its own looks fine.

## How to fix

Before:
```cpp
#include <cstdio>
void f() {
    FILE *f1 = fopen("a.txt", "w");
    FILE *f2 = fopen("a.txt", "r"); // <- 'a.txt' is already open for writing
}
```

After:
```cpp
#include <cstdio>
void f() {
    FILE *f1 = fopen("a.txt", "r");
    if (f1) fclose(f1);
}
```

## Related checkers

- [useClosedFile.md](useClosedFile.md), [readWriteOnlyFile.md](readWriteOnlyFile.md),
  [writeReadOnlyFile.md](writeReadOnlyFile.md), [IOWithoutPositioning.md](IOWithoutPositioning.md),
  [seekOnAppendedFile.md](seekOnAppendedFile.md) - other checks that follow the same `FILE*` through a
  function.
