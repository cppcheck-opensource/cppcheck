# clarifyCalculation

**Message**: Clarify calculation precedence for '*' and '?'.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

A calculation (`a * b`, `a + b`, ...) sits directly to the left of `?` in a ternary expression
(`a * b ? c : d`) - it's easy to misread this as `a * (b ? c : d)`, but it's actually `(a * b) ? c : d`.

## Motivation

The code compiles exactly as written and does exactly what the operator precedence rules say, but that's
not always what a quick read suggests - a reader can easily assume the ternary binds to the calculation's
last operand rather than to the whole calculation. Adding parentheses costs nothing and removes the
ambiguity for the next reader.

## How to fix

Add parentheses that make the actual grouping explicit.

Before:
```cpp
int f(char c) {
    return 10 * (c == 0) ? 1 : 2; // <- looks like '10 * ((c == 0) ? 1 : 2)'
}
```

After:
```cpp
int f(char c) {
    return (10 * (c == 0)) ? 1 : 2;
}
```

## Related checkers

- [clarifyStatement.md](clarifyStatement.md) - a different easy-to-misread-precedence pitfall, about
  `*p++;` rather than a calculation next to `?`.
