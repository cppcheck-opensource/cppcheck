# returnReference

**Message**: Reference to local variable returned.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C++

## Description

A function declared to return a reference returns a reference to a local variable.

## Motivation

A reference to a local variable becomes invalid the moment the function returns. Using the returned
reference afterwards is undefined behaviour, even though the memory is often still intact for a little
while, which is why this bug can appear to "work" in casual testing.

## How to fix

Before:
```cpp
int& foo() {
    int num = 2;
    return num; // <- returnReference
}
```

After:
```cpp
int foo() {
    int num = 2;
    return num;
}
```

## Related checkers

- [returnTempReference.md](returnTempReference.md) - the same idea, but returning a reference to a
  temporary object instead of a named local variable.
- [returnDanglingLifetime.md](returnDanglingLifetime.md) - the equivalent problem for a function that
  returns a pointer/iterator instead of a reference.
