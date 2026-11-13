# redundantAssignment

**Message**: Variable 'x' is reassigned a value before the old one has been used.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

A variable is given a value, but that value is completely overwritten by another assignment before
it's ever read.

## Motivation

This isn't wrong in the sense of producing an incorrect result today, but it signals that something
didn't happen the way the author intended: a value that was supposed to be used got clobbered first.
It's also a prime candidate for dead-code cleanup, and occasionally hides an outright bug (for example
a copy-pasted assignment that should have targeted a different variable).

## How to fix

Before:
```cpp
void f(int i) {
    i = 1;
    i = 1; // <- the first assignment is pointless
}
```

After:
```cpp
void f(int i) {
    i = 1;
}
```

## Related checkers

- [redundantAssignInSwitch.md](redundantAssignInSwitch.md) - the same idea, but across fall-through `switch` cases.
- [redundantInitialization.md](redundantInitialization.md) - the same idea, but for a variable's initializer value.
- [redundantCopyLocalConst.md](redundantCopyLocalConst.md) - a related redundant-copy check for `const` variables.
