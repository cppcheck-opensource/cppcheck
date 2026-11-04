# unreachableSwitchCase

**Message**: Switch case 'x' can never be selected because the switch condition is known to be 'y'.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

A `switch` is given a value cppcheck already knows, and a `case` label in it can never match that
value.

## Motivation

A `case` that can never be reached is dead code, and can indicate a mistake nearby - for example, that
the `switch`'s value was meant to still be variable at this point but has already been narrowed down by
an earlier check.

## How to fix

Before:
```cpp
enum T { A, B };
void f(const T &t) {
    if (t == A) {
        switch (t) {
        case A:
            break;
        case B: // <- 't' is known to be A here
            break;
        }
    }
}
```

After:
```cpp
enum T { A, B };
void f(const T &t) {
    switch (t) {
    case A:
        break;
    case B:
        break;
    }
}
```

## Related checkers

- [unreachableCode.md](unreachableCode.md) - code placed after an exit statement, rather than a `case` that can't be selected.
