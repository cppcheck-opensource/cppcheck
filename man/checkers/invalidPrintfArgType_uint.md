# invalidPrintfArgType_uint

**Message**: %u in format string (no. 1) requires 'unsigned int' but the argument type is 'signed int'<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Warning/Portability<br/>
**Language**: C/C++

## Description

The argument for an unsigned-integer specifier (`%u`, `%x`, `%o`, ...) doesn't match that specifier's
expected type/size (including the `h`/`hh`/`l`/`ll`/`z`/`j`/`t` length modifiers).

## Motivation

`printf` reads its arguments according to the type the format specifier implies, not the type the
argument actually has - a signedness or size mismatch means the value is read using the wrong number of
bytes or the wrong interpretation of the sign bit. Reading a variadic argument through a mismatched type
like this is undefined behaviour in its own right, not just a display quirk - the printed value being
nonsensical is only the most visible symptom.

## How to fix

Before:
```cpp
#include <cstdio>
void f(int x) {
    printf("%u", x); // <- needs 'unsigned int', not signed 'int'
}
```

After:
```cpp
#include <cstdio>
void f(unsigned int x) {
    printf("%u", x);
}
```

## Related checkers

- [invalidPrintfArgType_sint.md](invalidPrintfArgType_sint.md) - the signed-integer equivalent.
- [invalidPrintfArgType_float.md](invalidPrintfArgType_float.md) - the floating-point equivalent.
- [invalidScanfArgType_int.md](invalidScanfArgType_int.md) - the `scanf`-side equivalent.
