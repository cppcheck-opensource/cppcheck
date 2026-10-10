# invalidPrintfArgType_n

**Message**: %n in format string (no. 1) requires 'int \*' but the argument type is 'signed int'<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Warning/Portability<br/>
**Language**: C/C++

## Description

For a `printf`-family call, a `%n` specifier is given something that clearly isn't a writable pointer at
all - a plain integer, or a `const`-qualified pointer. It checks for *some* writable pointer, not
specifically a pointer to `int`, which is what `%n` actually requires.

## Motivation

`%n` writes the number of characters printed so far back through the pointer it's given - if the
argument isn't actually a pointer, this writes to whatever address that value happens to represent,
which is undefined behaviour (and is disabled outright by some platforms/libraries as a security risk).

## How to fix

Before:
```cpp
#include <cstdio>
void f(int x) {
    printf("%n", x); // <- needs an 'int *', not 'int'
}
```

After:
```cpp
#include <cstdio>
void f(int *x) {
    printf("%n", x);
}
```

## Related checkers

- [invalidPrintfArgType_s.md](invalidPrintfArgType_s.md) / [invalidPrintfArgType_p.md](invalidPrintfArgType_p.md) - the same idea for `%s` and `%p`.
