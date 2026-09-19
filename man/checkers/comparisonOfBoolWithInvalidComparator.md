# comparisonOfBoolWithInvalidComparator

**Message**: Comparison of a boolean value using relational operator (<, >, <= or >=).<br/>
**Category**: Code Quality<br/>
**Severity**: Warning<br/>
**Language**: C++

## Description

A boolean literal (`true`/`false`) is compared to something using `<`, `>`, `<=` or `>=`.

## Motivation

`<`, `>`, `<=` and `>=` are well-defined against a `bool` literal (`false` is `0`, `true` is `1`), so
this code already compiles and evaluates correctly - there is no functional problem to fix. The reason
to flag it is readability: `bool` only has two values, so an ordering comparison against `true`/`false`
says nothing that `==`/`!=` wouldn't say more directly, and it forces the reader to work out which of
`false`/`true` is "smaller" instead of just reading the equality check. Preferring `==`/`!=` for
two-valued types is the clearer, more idiomatic style.

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
