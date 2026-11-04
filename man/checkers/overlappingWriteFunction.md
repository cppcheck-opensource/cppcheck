# overlappingWriteFunction

**Message**: Overlapping read/write in memcpy() is undefined behavior<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C/C++

## Description

A call to a function like `memcpy()` is given source and destination ranges that overlap - only
`memmove()`-family functions are allowed to have overlapping ranges; `memcpy()` and similar are
undefined behaviour if the ranges overlap.

## Motivation

`memcpy()` (unlike `memmove()`) is free to copy in whatever order is fastest for the platform - forwards,
backwards, or in chunks - on the assumption that the source and destination never overlap. If they do
overlap, the copy can partially overwrite data it still needs to read, corrupting the result in a way
that can differ between compilers, optimization levels, or standard library implementations. cppcheck
only reports this when it can work out the exact size being copied; it does not warn just because two
ranges look suspiciously close together with a size it can't pin down.

## How to fix

Use `memmove()` instead of `memcpy()` when the source and destination ranges might overlap.

Before:
```cpp
#include <cstring>
void foo() {
    char a[10];
    memcpy(a, a+1, 2u); // <- source and destination overlap
}
```

After:
```cpp
#include <cstring>
void foo() {
    char a[10];
    memmove(a, a+1, 2u); // memmove() is defined to handle overlap safely
}
```

## Related checkers

- [overlappingWriteUnion.md](overlappingWriteUnion.md) - the same underlying overlapping-read/write
  hazard, but for reading and writing two overlapping members of a union in one expression instead of a
  function call.
