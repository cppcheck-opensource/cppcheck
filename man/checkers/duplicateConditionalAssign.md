# duplicateConditionalAssign

**Message**: Assignment 'x=5' is redundant with condition 'x==5'.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

An `if` assigns a value to a variable that the condition already guarantees it has (`if (x == 5) x = 5;`) -
the assignment has no effect.

## Motivation

If the condition already established what the variable equals, assigning that same value again inside
the `if` changes nothing - it's leftover code from an edit, or a sign the assignment was meant to use a
different value or variable.

This check may need `--check-level=exhaustive` to see every case.

## How to fix

Before:
```cpp
void f(int x) {
    if (x == 5)
        x = 5; // <- 'x' is already 5 here
}
```

After:
```cpp
void f(int x) {
    if (x == 5) {
    }
}
```

## Related checkers

- [knownConditionTrueFalse.md](knownConditionTrueFalse.md) - the more general check for a condition
  whose truth value cppcheck already knows in advance.
