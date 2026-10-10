# assignIfError

**Message**: Mismatching assignment and comparison, comparison 'y==1' is always false.<br/>
**Category**: Correctness<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

A variable that was just assigned via a bitmask operation (`x = y & 0xf0;`) is compared against a
constant that mask could never produce.

## Motivation

Once a variable has been masked with `& 0xf0`, its value is constrained to the multiples of 16 that mask
allows - comparing it against a value the mask rules out (like `1`) can never match, which usually means
either the mask or the comparison value is wrong.

## How to fix

Before:
```cpp
void f(int x) {
    int y = x & 0xf0;
    if (y == 1) {} // <- 'y' can never be 1, its low bits were just masked off
}
```

After:
```cpp
void f(int x) {
    int y = x & 0xf0;
    if (y == 0x10) {}
}
```

## Related checkers

- [knownConditionTrueFalse.md](knownConditionTrueFalse.md) - the more general check for a condition
  whose truth value cppcheck already knows in advance.
