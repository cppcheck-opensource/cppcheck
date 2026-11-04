# comparisonError

**Message**: Expression '(X & 0x7) == 0x8' is always false.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

A bitwise `&`/`|` expression is compared against a constant that mask could never actually produce.

## Motivation

Masking a value with `& 0x07` can only ever produce a result in `0..7`; comparing that result against
`8` (or any value the mask can't produce) is either dead code or a sign the mask or the compared-against
constant is wrong.

## How to fix

Before:
```cpp
void f(int a) {
    if ((a & 0x07) == 8) {} // <- masking with 0x07 can never produce 8
}
```

After:
```cpp
void f(int a) {
    if ((a & 0x0f) == 8) {}
}
```

## Related checkers

- [badBitmaskCheck.md](badBitmaskCheck.md) - `|` used where `&` was probably meant.
- [mismatchingBitAnd.md](mismatchingBitAnd.md) - chained `&` masks that share no bits.
