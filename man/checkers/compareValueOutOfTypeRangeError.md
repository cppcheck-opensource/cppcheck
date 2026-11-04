# compareValueOutOfTypeRangeError

**Message**: Comparing expression of type 'unsigned char' against value 256. Condition is always false.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

A variable is compared against a constant that lies outside the range its own type can represent (for
example, an `unsigned char` compared against `256`).

## Motivation

An `unsigned char` can never hold `256` - it wraps around before ever reaching it - so the comparison's
result is fixed regardless of the variable's actual value. This is usually a sign the variable's type is
too small for what it's meant to hold, or that the compared-against constant is wrong.

## How to fix

Before:
```cpp
void f(unsigned char c) {
    if (c == 256) {} // <- 'c' can never reach 256
}
```

After:
```cpp
void f(int c) {
    if (c == 256) {}
}
```

## Related checkers

- [knownConditionTrueFalse.md](knownConditionTrueFalse.md) - the more general check for a condition
  whose truth value cppcheck already knows in advance.
