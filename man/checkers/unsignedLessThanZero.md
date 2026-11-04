# unsignedLessThanZero and unsignedPositive

**Message**: Checking if unsigned expression 'x' is less than zero.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

An unsigned expression is compared against `0` in a way that can never be true (`unsignedLessThanZero`:
`x < 0`) or is always true (`unsignedPositive`: `x >= 0`), since an unsigned value can't be negative.

## Motivation

Both comparisons are tautological for an unsigned type, so the branch they guard either never runs or
always runs - not what the code visibly appears to be testing, and a common leftover from a variable
that used to be signed.

## How to fix

Before:
```cpp
void foo(unsigned int x) {
  if (x < 0) {} // <- can never be true
}
```

After:
```cpp
void foo(int x) {
  if (x < 0) {}
}
```

Before:
```cpp
void foo() {
  for(unsigned char i = 10; i >= 0; i--) {} // <- always true, infinite loop risk
}
```

After:
```cpp
void foo() {
  for(int i = 10; i >= 0; i--) {}
}
```

## Related checkers

- [pointerLessThanZero.md](pointerLessThanZero.md) - the same idea, for a pointer compared against `0`
  instead of an unsigned integer.
