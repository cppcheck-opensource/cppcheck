# uselessAssignmentArg and uselessAssignmentPtrArg

**Message**: Assignment of function parameter has no effect outside the function.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

A function parameter (plain or pointer) is assigned a new value that is never used afterwards - since
parameters are passed by value in C/C++, this change is invisible to the caller and has no effect.

- `uselessAssignmentArg`: the parameter is an ordinary by-value parameter.
- `uselessAssignmentPtrArg`: the parameter is a pointer itself being reassigned (not what it points to).

## Motivation

Assigning to a by-value parameter without reading it again afterwards looks like it's meant to
communicate something back to the caller, but it can't - parameters are local copies. This usually
means either the assignment is pointless leftover code, or the author actually needed a pointer/
reference parameter (or a return value) to get the result back out.

## How to fix

Before:
```cpp
void foo(int b) {
    b = 5; // <- uselessAssignmentArg: caller never sees this
}
```

After: change the return type instead of the parameter, or drop the assignment.
```cpp
int foo() {
    return 5;
}
```

