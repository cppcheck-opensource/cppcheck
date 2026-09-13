# comparisonOfBoolWithBoolError

**Message**: Comparison of a variable having boolean value using relational (<, >, <= or >=) operator.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C++

## Description

Two `bool` variables are compared with `<`, `>`, `<=` or `>=`.

## Motivation

`bool` only has two values, so ordering two of them doesn't express anything `==`/`!=` wouldn't say
more clearly, and is easy to get backwards since `false < true` is not always the intuitive direction a
reader expects.

## How to fix

Before:
```cpp
void f(bool a, bool b) {
    if (a < b) {} // <- relational comparison between two bools
}
```

After:
```cpp
void f(bool a, bool b) {
    if (a != b) {}
}
```

## Related checkers

- [comparisonOfBoolWithInvalidComparator.md](comparisonOfBoolWithInvalidComparator.md) - the same kind
  of relational comparison, but against a `true`/`false` literal instead of another `bool` variable.
