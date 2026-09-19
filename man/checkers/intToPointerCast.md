# intToPointerCast

**Message**: Casting non-zero decimal integer literal to pointer.<br/>
**Category**: Portability<br/>
**Severity**: Portability<br/>
**Language**: C/C++

## Description

A non-zero integer literal (not a hex address like `0x7000`, which is common in embedded code) is cast
directly to a pointer.

## Motivation

A decimal, octal, or binary integer literal cast to a pointer looks like it's meant to be a specific
memory address, but writing addresses in those bases is unusual and easy to mistype or misread compared
to the conventional hexadecimal form - a small typo can silently produce a very different address.

## How to fix

Write the address in hexadecimal, which is the conventional and clearer way to express a raw address.

Before:
```cpp
#include <cstdint>
uint8_t* f() {
    uint8_t* ptr = (uint8_t*)7; // <- 7 isn't a real address
    return ptr;
}
```

After:
```cpp
#include <cstdint>
uint8_t* f() {
    uint8_t* ptr = (uint8_t*)0x7000; // an actual address is fine
    return ptr;
}
```

## Related checkers

- [invalidPointerCast.md](invalidPointerCast.md) - a different pointer-cast portability issue, about
  casting between two pointer types whose values aren't laid out the same way in memory.
