# invalidPrintfArgType_s

**Message**: %s in format string (no. 1) requires 'char \*' but the argument type is 'signed int'<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Warning/Portability<br/>
**Language**: C/C++

## Description

For a `printf`-family call, a `%s` specifier isn't given a `char*`.

## Motivation

`printf` reads a `%s` argument as a pointer to a null-terminated string - if the argument is actually a
plain integer (or any other non-pointer value), `printf` dereferences whatever address that value
happens to represent, which is undefined behaviour.

## How to fix

Before:
```cpp
#include <cstdio>
void f(int x) {
    printf("%s", x); // <- needs a 'char *', not 'int'
}
```

After:
```cpp
#include <cstdio>
void f(const char *x) {
    printf("%s", x);
}
```

## Related checkers

- [invalidScanfArgType_s.md](invalidScanfArgType_s.md) - the `scanf`-side equivalent.
- [invalidPrintfArgType_n.md](invalidPrintfArgType_n.md) / [invalidPrintfArgType_p.md](invalidPrintfArgType_p.md) - the same idea for `%n` and `%p`.
