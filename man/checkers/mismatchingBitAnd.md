# mismatchingBitAnd

**Message**: Mismatching bitmasks. Result is always 0 (X = Y & 0xf0; Z = X & 0x1; => Z=0).<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

A variable is masked with `& 0xf0`, and the result is masked again later with a value that shares no
bits with the first mask - so the final result is always `0`.

## Motivation

Chaining two bitmasks that share no bits is either dead code (the second mask can be removed since the
result is always `0`) or a sign that one of the two masks is wrong - either way it's worth a second
look.

## How to fix

Before:
```cpp
void f(int x) {
    int a = x & 0xf0;
    int b = a & 0x1; // <- always 0, no bits are shared with 0xf0
    if (b) {}
}
```

After:
```cpp
void f(int x) {
    int a = x & 0xf0;
    int b = a & 0x10;
    if (b) {}
}
```

## Related checkers

- [badBitmaskCheck.md](badBitmaskCheck.md) - `|` used where `&` was probably meant.
- [comparisonError.md](comparisonError.md) - a bitwise expression compared against a constant it can
  never produce.
