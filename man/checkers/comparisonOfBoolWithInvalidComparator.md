# comparisonOfBoolWithInvalidComparator

**Message**: Comparison of a boolean value using relational operator (<, >, <= or >=).<br/>
**Category**: Code Quality<br/>
**Severity**: Warning<br/>
**Language**: C++

## Description

A boolean literal (`true`/`false`) is compared to something using `<`, `>`, `<=` or `>=`.

## Motivation

`bool` only has two values, so ordering comparisons against a literal `true`/`false` don't express
anything an equality comparison (`==`/`!=`) wouldn't say more clearly - and are easy to get backwards,
since `false < true` is not always the intuitive direction a reader expects.

## How to fix

Before:
```cpp
void f(bool x) {
    if (x > false) {} // <- relational comparison against a bool literal
}
```

After:
```cpp
void f(bool x) {
    if (x == true) {}
}
```

## Related checkers

- [comparisonOfBoolWithBoolError.md](comparisonOfBoolWithBoolError.md) - the same kind of relational
  comparison, but between two `bool` variables instead of against a literal.
