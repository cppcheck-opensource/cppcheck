# duplicateExpression

**Message**: Same expression on both sides of '=='.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

The exact same expression appears on both sides of a comparison or other binary operator (`a == a`,
`a - a`, ...), or more than once in a chain of the same operator.

## Motivation

Comparing or combining an expression with an identical copy of itself always produces the same,
predetermined result (`true`, `false`, or zero) regardless of the expression's actual value - this is
almost always a typo for what should have been two different variables.

## How to fix

Before:
```cpp
void foo(int a) {
    if (a == a) { } // <- always true
}
```

After:
```cpp
void foo(int a, int b) {
    if (a == b) { }
}
```

## Related checkers

- [oppositeExpression.md](oppositeExpression.md) - the mirror-image mistake, comparing an expression against its logical/arithmetic opposite.
- [duplicateAssignExpression.md](duplicateAssignExpression.md) - the same idea across two separate variable assignments.
