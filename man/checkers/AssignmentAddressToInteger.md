# AssignmentAddressToInteger and AssignmentIntegerToAddress

**Message**: Assigning a pointer to an integer is not portable.<br/>
**Category**: Portability<br/>
**Severity**: Portability<br/>
**Language**: C/C++

## Description

Code silently narrows a pointer (address) down to a plain integer type, or the other way around, via a
plain assignment. On platforms where `sizeof(void*) != sizeof(int)` (most notably 64-bit platforms,
where a pointer is 8 bytes and `int` is usually still 4 bytes) this loses information: part of the
address is silently discarded.

- `AssignmentAddressToInteger`: a pointer value is assigned to a plain integer variable, for example
  `int i = p;`.
- `AssignmentIntegerToAddress`: a plain integer value is assigned to a pointer variable, for example
  `int *p = i;`.

This checks `char`/`short`/`int` variables (not `long`/`long long`, and not `bool`, which is a common,
intentional null-check idiom rather than a truncation bug). This checker only runs when the
`portability` severity is enabled.

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
int foo(int *p) {
    int a = p; // <- AssignmentAddressToInteger
    return a;
}
```

After:
```cpp
#include <cstdint>
intptr_t foo(int *p) {
    intptr_t a = reinterpret_cast<intptr_t>(p);
    return a;
}
```

## Related checkers

- [CastAddressToIntegerAtReturn.md](CastAddressToIntegerAtReturn.md) - the same idea, but for a
  function `return` rather than a plain assignment.
