# moduloAlwaysTrueFalse

**Message**: Comparison of modulo result is predetermined, because it is always less than 5.<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

A `%` result is compared against a constant outside the range that modulo operation could ever produce.

## Motivation

`x % 5` can only ever be `0..4` - comparing it against `5` or anything larger makes the comparison
predetermined, which usually means the modulo divisor or the compared-against constant is wrong.

## How to fix

Before:
```cpp
void f(int x) {
    if (x % 5 == 5) {} // <- 'x % 5' is always 0..4
}
```

After:
```cpp
void f(int x) {
    if (x % 5 == 0) {}
}
```

## Related checkers

- [knownConditionTrueFalse.md](knownConditionTrueFalse.md) - the more general check for a condition
  whose truth value cppcheck already knows in advance.
