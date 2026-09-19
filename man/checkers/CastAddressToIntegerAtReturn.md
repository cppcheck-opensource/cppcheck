# CastAddressToIntegerAtReturn and CastIntegerToAddressAtReturn

**Message**: Returning an address value in a function with integer return type is not portable.<br/>
**Category**: Portability<br/>
**Severity**: Portability<br/>
**Language**: C/C++

## Description

A function silently narrows a pointer (address) down to a plain integer return type, or the other way
around. On platforms where `sizeof(void*) != sizeof(int)` (most notably 64-bit platforms, where a
pointer is 8 bytes and `int` is usually still 4 bytes) this loses information: part of the address is
silently discarded.

- `CastAddressToIntegerAtReturn`: a function with an integer return type returns a pointer value, for
  example `int foo(char *p) { return p; }`.
- `CastIntegerToAddressAtReturn`: a function with a pointer return type returns a plain integer value,
  for example `void* foo(int i) { return i; }`.

This checks `char`/`short`/`int` (not `long`/`long long`, and not `bool`, which is a common,
intentional idiom rather than a truncation bug), and only when analyzing for a 64-bit target - on a
32-bit target a pointer and an `int` are the same width, so returning one as the other isn't a
portability problem there. This checker only runs when the `portability` severity is enabled.

## Motivation

Storing an address in a type that is narrower than a pointer is not portable: it works by accident on
platforms where the two types happen to be the same width, and silently truncates the address (or
sign-extends a small integer into a bogus address) on platforms where they are not, most notably when
porting 32-bit code to 64-bit.

## How to fix

Use a pointer type, or an integer type explicitly meant to hold a pointer (`intptr_t`/`uintptr_t` from
`<cstdint>`), instead of a plain `int`/`char`/etc.

Before:
```cpp
void* foo(int i) {
    return i; // <- CastIntegerToAddressAtReturn
}
```

After:
```cpp
#include <cstdint>
void* foo(intptr_t i) {
    return reinterpret_cast<void*>(i);
}
```

## Related checkers

- [AssignmentAddressToInteger.md](AssignmentAddressToInteger.md) - the same idea, but for a plain
  assignment rather than a function `return`.
