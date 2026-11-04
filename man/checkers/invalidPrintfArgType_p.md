# invalidPrintfArgType_p

**Message**: %p in format string (no. 1) requires an address but the argument type is 'signed int'<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Warning/Portability<br/>
**Language**: C/C++

## Description

For a `printf`-family call, a `%p` specifier isn't given a pointer.

## Motivation

`%p` is meant to print the value of a pointer. Passing a non-pointer argument means `printf` reads it as
if it were a pointer-sized value from the variadic argument list, which is undefined behaviour the
moment that mismatched read happens - not just a meaningless value, since on some calling conventions it
also reads the wrong argument size entirely, throwing off every argument read after it.

## How to fix

Before:
```cpp
#include <cstdio>
void f(int x) {
    printf("%p", x); // <- needs an address, not 'int'
}
```

After:
```cpp
#include <cstdio>
void f(void *x) {
    printf("%p", x);
}
```

## Related checkers

- [invalidPrintfArgType_s.md](invalidPrintfArgType_s.md) / [invalidPrintfArgType_n.md](invalidPrintfArgType_n.md) - the same idea for `%s` and `%n`.
