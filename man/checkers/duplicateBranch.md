# duplicateBranch

**Message**: Found duplicate branches for 'if' and 'else'.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

The `if` and `else` branches of a statement contain identical code, so the condition makes no
difference to what runs.

## Motivation

If both branches do the same thing, the condition is pointless - either the branches were meant to
differ and one was copy-pasted by mistake, or the `if`/`else` itself can be removed entirely.

## How to fix

Before:
```cpp
void f(int a, int &b) {
    if (a)
        b = 1;
    else
        b = 1; // <- identical to the 'if' branch
}
```

After:
```cpp
void f(int a, int &b) {
    if (a)
        b = 1;
    else
        b = 2;
}
```

## Related checkers

- [duplicateExpression.md](duplicateExpression.md) - the same "is this a copy-paste mistake?" idea, for an expression instead of a whole branch.
