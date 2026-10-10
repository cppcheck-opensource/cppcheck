# incorrectLogicOperator

**Message**: Logical disjunction always evaluates to true: x != 1 || x != 2.<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

Two comparisons on the same variable, joined by `&&`/`||`, combine to something that's always true or
always false, regardless of the variable's actual value.

## Motivation

`x == 1 && x == 2` can never be true (a variable can't equal both 1 and 2 at once), and
`x != 1 || x != 2` can never be false (it's always at least one of those). Writing this is a strong
signal the wrong logical operator was used - `&&` instead of `||`, or vice versa - or that one of the
compared values is wrong.

This check may need `--check-level=exhaustive` to see every case.

## How to fix

Before:
```cpp
void f(int x) {
    if (x == 1 && x == 2) {} // <- can never be true
}
```

After:
```cpp
void f(int x) {
    if (x == 1 || x == 2) {}
}
```

## Related checkers

- [redundantCondition.md](redundantCondition.md) - the same style of two-comparisons-on-one-variable
  analysis, but for when one comparison is already implied by the other rather than making the whole
  expression always true/false.
