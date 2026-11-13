# invalidPrintfArgType_float

**Message**: %f in format string (no. 1) requires 'double' but the argument type is 'signed int'<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Warning/Portability<br/>
**Language**: C/C++

## Description

The argument for a floating-point specifier (`%f`, `%e`, `%g`, ...) isn't a floating-point value.

## Motivation

Because `printf` is variadic, a `float` argument is promoted to `double` and read back out as
`double` by the corresponding specifier - passing an integer instead means `printf` reads the wrong
number of bytes and reinterprets them as a floating-point value. That mismatched read is undefined
behaviour in its own right, not just a meaningless printed number.

## How to fix

Before:
```cpp
#include <cstdio>
void f(int x) {
    printf("%f", x); // <- needs 'double', not 'int'
}
```

After:
```cpp
#include <cstdio>
void f(double x) {
    printf("%f", x);
}
```

## Related checkers

- [invalidPrintfArgType_uint.md](invalidPrintfArgType_uint.md) / [invalidPrintfArgType_sint.md](invalidPrintfArgType_sint.md) - the integer equivalents.
- [invalidScanfArgType_float.md](invalidScanfArgType_float.md) - the `scanf`-side equivalent.
