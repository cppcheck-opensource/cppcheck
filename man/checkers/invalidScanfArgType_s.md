# invalidScanfArgType_s

**Message**: %s in format string (no. 1) requires a 'char \*' but the argument type is 'signed int \*'<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Warning/Portability<br/>
**Language**: C/C++

## Description

For a `scanf`-family call, the argument for a `%s`/`%[...]` specifier isn't a pointer to the matching
type (remember every `scanf` conversion writes through a pointer).

## Motivation

`scanf` writes the characters it reads directly through the pointer it's given - if that pointer
doesn't actually point to a `char`/`wchar_t` buffer, the write corrupts whatever memory it does point
to. Writing through a pointer typed differently than the object it actually points to is undefined
behaviour in its own right, not just a corrupted value.

## How to fix

Before:
```cpp
#include <cstdio>
void f(int x) {
    scanf("%s", &x); // <- needs a 'char *', not 'int *'
}
```

After:
```cpp
#include <cstdio>
void f() {
    char x[32];
    scanf("%31s", x);
}
```

## Related checkers

- [invalidScanfArgType_int.md](invalidScanfArgType_int.md) / [invalidScanfArgType_float.md](invalidScanfArgType_float.md) - the same idea for numeric `scanf` conversions.
- [invalidPrintfArgType_s.md](invalidPrintfArgType_s.md) - the `printf`-side equivalent for `%s`.
