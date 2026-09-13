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

`bool` only has two values, so ordering its result doesn't express anything `==`/`!=` wouldn't say more
clearly, and is easy to get backwards since `false < true` is not always the intuitive direction a
reader expects.

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

