# incompleteArrayFill

**Message**: Array 'a[5]' might be filled incompletely. Did you forget to multiply the size given to 'memset()' with 'sizeof(*a)'?<br/>
**Category**: Correctness<br/>
**Severity**: Warning/Portability<br/>
**Language**: C/C++

## Description

`memset()`/`memcpy()`/`memmove()` is given a byte count equal to the *number of elements* in an array,
rather than their combined size in bytes - unless each element happens to be exactly one byte, this only
fills part of the array. When the array's element type is specifically `bool`, this is reported at
`portability` severity instead: `sizeof(bool)` happens to be `1` on most common platforms (so the code
"works" there), but the C/C++ standard doesn't guarantee it, so the exact same code is a real, silent
bug on a platform where `bool` is wider.

## Motivation

These functions take their size argument in bytes, not element count. Passing the element count directly
is an easy mistake - it compiles fine, and on typical platforms with 1-byte types it can even happen to
be correct, but for any array element wider than one byte it silently leaves most of the array
untouched.

## How to fix

Multiply the element count by `sizeof(*array)` (or the element type's size) to get the correct byte
count.

Before:
```cpp
#include <cstring>
void f() {
    int a[5];
    memset(a, 123, 5); // <- fills 5 bytes, not 5 ints
}
```

After:
```cpp
#include <cstring>
void f() {
    int a[5];
    memset(a, 123, 5 * sizeof(*a));
}
```
