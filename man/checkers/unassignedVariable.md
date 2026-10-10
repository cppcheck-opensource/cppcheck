# unassignedVariable

**Message**: Variable 'i' is not assigned a value.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

A variable is read (or its address is taken) without ever having been given a value first.

## Motivation

Reading a variable that's never been assigned is a sign of a bug: a forgotten initialization, or a
variable name that was meant to refer to something else. (Note this checker flags the code pattern
itself; see [uninitvar.md](uninitvar.md) for the related, more precise check on genuinely reading an
uninitialized value.)

## How to fix

Give the variable a value before reading it.

Before:
```cpp
int foo() {
    int i; // <- read below without ever being assigned
    return i;
}
```

After:
```cpp
int foo() {
    int i = 0;
    return i;
}
```

## Related checkers

- [uninitvar.md](uninitvar.md) - the closely related, more precise family of checks specifically about
  reading an uninitialized value.
- [unusedVariable.md](unusedVariable.md) and [unreadVariable.md](unreadVariable.md) - the sibling
  findings for a variable that's never used at all, or assigned but never read.
