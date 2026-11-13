# duplicateAssignExpression

**Message**: Same expression used in consecutive assignments of 'i' and 'j'.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

Two variables, declared one after another, are assigned the exact same expression - often a copy-paste
mistake where the second line should refer to something else.

## Motivation

Two variables initialized to the same expression right next to each other is a common outcome of
copy-pasting one line and forgetting to change one of the operands - if it's not a mistake, computing
the same value twice (rather than reusing it) is also wasteful.

## How to fix

Before:
```cpp
int f() __attribute__((pure));
int g() __attribute__((pure));
void test() {
    int i = f();
    int j = f(); // <- same expression as 'i', is this a copy-paste mistake?
}
```

After:
```cpp
int f() __attribute__((pure));
int g() __attribute__((pure));
void h(int, int);
void test() {
    int i = f();
    int j = g();
    h(i, j);
}
```

## Related checkers

- [duplicateExpression.md](duplicateExpression.md) - the same idea for one expression compared against itself, rather than two separate assignments.
