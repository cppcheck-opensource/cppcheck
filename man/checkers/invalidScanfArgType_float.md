# invalidScanfArgType_float

**Message**: %f in format string (no. 1) requires 'float \*' but the argument type is 'signed int \*'<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Warning/Portability<br/>
**Language**: C/C++

## Description

For a `scanf`-family call, the argument for a floating-point specifier (`%f`, `%e`, `%g`, ...) isn't a
pointer to the matching floating-point type.

## Motivation

`scanf` writes the parsed floating-point value through the pointer it's given, in the binary
floating-point representation that pointer's type implies - a mismatched pointer type (for example an
`int*`) means the write is the wrong size and reinterprets those bytes incorrectly. Writing through a
pointer typed differently than the object it actually points to is undefined behaviour in its own
right, not just a corrupted value.

## How to fix

Before:
```cpp
#include <cstdio>
void f(int *x) {
    scanf("%f", x); // <- needs a 'float *', not 'int *'
}
```

After:
```cpp
#include <cstdio>
void f(float *x) {
    scanf("%f", x);
}
```

## Related checkers

- [invalidScanfArgType_s.md](invalidScanfArgType_s.md) / [invalidScanfArgType_int.md](invalidScanfArgType_int.md) - the same idea for string and integer `scanf` conversions.
- [invalidPrintfArgType_float.md](invalidPrintfArgType_float.md) - the `printf`-side equivalent.
