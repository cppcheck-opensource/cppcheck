# moduloofone

**Message**: Modulo of one is always equal to zero<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

An expression is reduced modulo `1`, which is always `0` no matter what the other operand is.

## Motivation

`x % 1` is always `0` for any integer `x` - the computation is pointless and almost always signals a
typo (a `1` that should have been some other number, or a variable that should have been used instead
of a literal `1`).

## How to fix

Before:
```cpp
void f(unsigned int x) {
  int y = x % 1; // <- always 0
}
```

After:
```cpp
void f(unsigned int x) {
  int y = x % 2;
  (void)y;
}
```
