# sizeofCalculation

**Message**: Found calculation inside sizeof().<br/>
**Category**: Correctness<br/>
**Severity**: Warning (inconclusive)<br/>
**Language**: C/C++

## Description

An arithmetic/increment/decrement calculation appears inside `sizeof`, e.g. `sizeof(a + b)` or
`sizeof(x++)` - it looks like it runs, but it never does.

## Motivation

The operand of `sizeof` is normally not evaluated at all - only its type is used to compute the result.
A calculation written inside `sizeof` reads as if it executes (especially `x++`, which looks like it
must have a side effect), but it silently never runs, which is confusing for anyone who later expects
that side effect to have happened.

## How to fix

Before:
```cpp
void f(int a, int b) {
    int s = sizeof(a + b); // <- the addition never actually happens
}
```

After:
```cpp
void f(int a, int b) {
    int s = sizeof(a);
}
```
