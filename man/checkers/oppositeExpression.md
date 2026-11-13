# oppositeExpression

**Message**: Opposite expression on both sides of '&&'.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

An expression and its logical/arithmetic opposite (`a` and `!a`, `x` and `-x`, ...) appear on both
sides of the same operator - for `&&`/`||` in particular, this makes the whole condition always
false/true.

## Motivation

Combining an expression with its own negation can never depend on the expression's actual value - the
result is fixed in advance, which usually means one side was meant to refer to something else.

## How to fix

Before:
```cpp
void f(bool a) {
    if (a && !a) {} // <- always false
}
```

After:
```cpp
void f(bool a, bool b) {
    if (a && !b) {}
}
```

## Related checkers

- [duplicateExpression.md](duplicateExpression.md) - the mirror-image mistake, comparing an expression against an identical copy of itself.
