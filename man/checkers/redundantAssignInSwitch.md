# redundantAssignInSwitch

**Message**: Variable 'x' is reassigned a value before the old one has been used.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

A variable is assigned a value in one `switch` `case`, and that value is overwritten by another
assignment in a later `case` that it falls through into, without the value ever being read in between.

## Motivation

This is the `switch`-fallthrough flavor of a redundant assignment, and it's usually a sign of a missing
`break;` rather than an intentional fallthrough - if the fallthrough really is intentional, the first
assignment is still pointless and worth removing for clarity.

## How to fix

Before:
```cpp
void bar(int);
void foo(int a) {
    int y = 1;
    switch (a) {
    case 2:
        y = 2; // <- falls through into case 3, missing 'break;'?
    case 3:
        y = 3;
    }
    bar(y);
}
```

After:
```cpp
void bar(int);
void foo(int a) {
    int y = 1;
    switch (a) {
    case 2:
        y = 2;
        break;
    case 3:
        y = 3;
    }
    bar(y);
}
```

## Related checkers

- [redundantAssignment.md](redundantAssignment.md) - the same idea outside of a `switch`.
- [redundantBitwiseOperationInSwitch.md](redundantBitwiseOperationInSwitch.md) - the bitwise-assignment equivalent in a `switch`.
