# terminateStrncpy

**Message**: The buffer 'dest' may not be null-terminated after the call to strncpy().<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Warning (inconclusive)<br/>
**Language**: C/C++

## Description

`strncpy()` is called with a length equal to (or larger than) the destination buffer's size, and the
source may be at least that long - in that case `strncpy()` does not null-terminate the destination, so
it may not be a valid C string afterwards. Since this is a "may not be null-terminated" finding rather
than a certainty, it is only reported when `--inconclusive` is enabled.

## Motivation

`strncpy()` only writes a trailing `'\0'` if the source string is shorter than the given length; if the
source is at least as long as the length, the destination is left completely full with no terminator.
Any later code that treats the buffer as a normal null-terminated C string then reads past its end.

## How to fix

Before:
```cpp
#include <cstring>
void f() {
    char dest[10];
    strncpy(dest, "abcdefghijklmnop", sizeof(dest)); // <- may not be null-terminated
}
```

After:
```cpp
#include <cstring>
void f() {
    char dest[10];
    strncpy(dest, "abcdefghijklmnop", sizeof(dest) - 1);
    dest[sizeof(dest) - 1] = '\0';
}
```
