# invalidFunctionArg

**Message**: Invalid $symbol() argument nr 1. The value is 0 but the valid values are '1:'.<br/>
**Category**: Correctness<br/>
**Severity**: Error/Warning<br/>
**Language**: C/C++

## Description

An argument value falls outside the numeric range a function actually accepts, based on what cppcheck
knows about the function (from a library configuration) and about the value passed.

## Motivation

Many standard library functions only behave correctly for a specific range of an argument (a base for
`strtol()`, a mode for a file function, ...); passing a value outside that range is undefined or
implementation-defined behaviour that a compiler will not catch.

## How to fix

Before:
```cpp
void f(char *a, char **b) {
    strtol(a, b, 1); // <- base 1 is not valid
}
```

After:
```cpp
void f(char *a, char **b) {
    strtol(a, b, 10);
}
```

## Related checkers

- [invalidFunctionArgBool.md](invalidFunctionArgBool.md) - the same idea, but for an argument that
  needs a real boolean rather than a plain `0`/`1`.
- [invalidFunctionArgStr.md](invalidFunctionArgStr.md) - the same idea, but for an argument that needs
  a null-terminated C-string.
