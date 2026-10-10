# invalidScanfArgType_int

**Message**: %d in format string (no. 1) requires 'int \*' but the argument type is 'float \*'<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Warning/Portability<br/>
**Language**: C/C++

## Description

For a `scanf`-family call, the argument for an integer specifier (`%d`, `%u`, `%x`, ...) isn't a
pointer to the matching integer type (including the `h`/`hh`/`l`/`ll`/`z`/`j`/`t` length modifiers).

## Motivation

`scanf` writes the parsed integer through the pointer it's given, using exactly as many bytes as the
specifier's type implies - a mismatched pointer type means the write is the wrong size for what it
actually points to. Writing through a pointer typed differently than the object it actually points to is
undefined behaviour in its own right, not just a matter of corrupting adjacent memory or only partially
updating the intended variable.

## How to fix

Before:
```cpp
#include <cstdio>
void f(float *x) {
    scanf("%d", x); // <- needs an 'int *', not 'float *'
}
```

After:
```cpp
#include <cstdio>
void f(int *x) {
    scanf("%d", x);
}
```

## Related checkers

- [invalidScanfArgType_s.md](invalidScanfArgType_s.md) / [invalidScanfArgType_float.md](invalidScanfArgType_float.md) - the same idea for string and floating-point `scanf` conversions.
- [invalidPrintfArgType_uint.md](invalidPrintfArgType_uint.md) / [invalidPrintfArgType_sint.md](invalidPrintfArgType_sint.md) - the `printf`-side equivalents.
