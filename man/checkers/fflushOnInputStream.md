# fflushOnInputStream

**Message**: fflush() called on input stream 'x' may result in undefined behaviour on non-linux systems.<br/>
**Category**: Portability<br/>
**Severity**: Portability<br/>
**Language**: C/C++

## Description

`fflush()` is called on an input stream (`stdin`, or a file opened for reading).

## Motivation

The C standard only defines `fflush()`'s effect for output streams; on an input stream it's
implementation-defined - some platforms discard unread input as a convenience, others don't define
anything useful, so relying on it isn't portable.

## How to fix

Before:
```cpp
#include <cstdio>
void f() {
    fflush(stdin); // <- undefined/implementation-defined on an input stream
}
```

After:
```cpp
#include <cstdio>
void f() {
    while (getchar() != '\n') {}
}
```
