# redundantBitwiseOperationInSwitch

**Message**: Redundant bitwise operation on 'x' in 'switch' statement. 'break;' missing?<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

The same bitwise assignment (`x |= 1`, `x &= 1`, ...) is applied to a variable twice across
fall-through `switch` cases, with nothing reading the value in between.

## Motivation

Just like a redundant plain assignment, this is usually a sign of a missing `break;` between two
`case` labels - the first bitwise operation has no effect once the identical one runs again in the
next case.

## How to fix

Before:
```cpp
void foo(int a) {
    int y = 1;
    switch (a) {
    case 2:
        y |= 3; // <- same operation repeated in case 3
    case 3:
        y |= 3;
        break;
    }
}
```

After:
```cpp
void foo(int a) {
    int y = 1;
    switch (a) {
    case 2:
        y |= 3;
        break;
    case 3:
        y |= 3;
        break;
    }
}
```

## Related checkers

- [redundantAssignInSwitch.md](redundantAssignInSwitch.md) - the plain-assignment equivalent in a `switch`.
