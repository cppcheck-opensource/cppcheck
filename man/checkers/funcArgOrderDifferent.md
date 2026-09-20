# funcArgOrderDifferent

**Message**: Function 'func2' argument order different: declaration 'a, b, c' definition 'c, b, a'<br/>
**Category**: Code Quality<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

A function's declaration and its definition give the same set of argument names, but in a different
order - a strong sign that a maintenance edit swapped two parameters in only one of the two places.

## Motivation

If the declaration and definition disagree about which parameter is which, calls written against the
declaration pass arguments in the wrong logical order relative to what the definition actually does with
them - a real, easy-to-miss bug, not just a style nit.

## How to fix

Before:
```cpp
void func2(int a, int b, int c);
void func2(int c, int b, int a) { } // <- 'a' and 'c' swapped places
```

After:
```cpp
void func2(int a, int b, int c);
void func2(int a, int b, int c) { }
```

## Related checkers

- [funcArgNamesDifferent.md](funcArgNamesDifferent.md) - the same kind of declaration/definition
  mismatch, but for an argument name changing (or disappearing) rather than two arguments swapping order.
