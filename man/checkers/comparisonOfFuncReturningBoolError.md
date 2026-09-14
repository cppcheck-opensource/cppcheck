# comparisonOfFuncReturningBoolError and comparisonOfTwoFuncsReturningBoolError

**Message**: Comparison of a function returning boolean value using relational (<, >, <= or >=) operator.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C++

## Description

The result of a function known to return `bool` is compared with `<`, `>`, `<=` or `>=` -
`comparisonOfFuncReturningBoolError` when one side is such a call, `comparisonOfTwoFuncsReturningBoolError`
when both sides are.

## Motivation

`<`, `>`, `<=` and `>=` are well-defined for `bool` results (`false` is `0`, `true` is `1`), so this code
already compiles and evaluates correctly - there is no functional problem to fix. The reason to flag it
is readability: `bool` only has two values, so ordering the result of a bool-returning function says
nothing that `==`/`!=` wouldn't say more directly, and it forces the reader to work out which of
`false`/`true` is "smaller" instead of just reading the equality/logical check. Preferring `==`/`!=` (or
plain `&&`/`!`) for two-valued results is the clearer, more idiomatic style.

## How to fix

Before:
```cpp
bool compare1(int x);
bool compare2(int x);
void f(int x) {
    if (compare1(x) > compare2(x)) {} // <- relational comparison between two bool-returning calls
}
```

After:
```cpp
bool compare1(int x);
bool compare2(int x);
void f(int x) {
    if (compare1(x) && !compare2(x)) {}
}
```

