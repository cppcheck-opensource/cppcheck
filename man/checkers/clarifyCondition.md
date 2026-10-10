# clarifyCondition

**Message**: Suspicious condition (assignment + comparison); Clarify expression with parentheses.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

An assignment or a bitwise operator sits next to a comparison in a way whose precedence is easy to
misread (`x = a < 0`, `a & b == c`) - cppcheck suggests adding parentheses to make the intended meaning
explicit.

## Motivation

`x = a < 0` really means `x = (a < 0)` (assign a boolean), not `(x = a) < 0` - but a reader skimming the
line can easily assume the opposite. Adding parentheses costs nothing and removes any ambiguity about
which reading was intended.

## How to fix

Before:
```cpp
void f(int a) {
    int x;
    if (x = a < 0) {} // <- is this '(x = a) < 0' or 'x = (a < 0)'?
}
```

After:
```cpp
void f(int a) {
    int x;
    x = (a < 0);
    if (x) {}
}
```

## Related checkers

- [assignmentInCondition.md](assignmentInCondition.md) - a related but distinct mistake: a container or
  iterator assignment written where a comparison was probably meant.
