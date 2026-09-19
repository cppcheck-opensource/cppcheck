# badBitmaskCheck

**Message**: Result of operator '|' is always true if one operand is non-zero. Did you intend to use '&'?<br/>
**Category**: Correctness<br/>
**Severity**: Warning/Style<br/>
**Language**: C/C++

## Description

A boolean/condition value is computed with `|` where `&` was probably meant - the result of `x | mask`
is true as soon as `x` is nonzero, regardless of `mask`. This message is also used for the opposite,
harmless mistake: an operand `| 0` that has no effect and can be removed (reported at `style` severity
instead of `warning`).

## Motivation

`|` and `&` look similar but behave very differently in a boolean context: `x | mask` is true whenever
`x` alone is nonzero, so the mask contributes nothing and the check silently always passes. This is easy
to miss because the code compiles and often "looks right" at a glance.

This check may need `--check-level=exhaustive` to see every case.

## How to fix

Before:
```cpp
bool f(int x) {
    bool b = x | 0x02; // <- always true if x is nonzero
    return b;
}
```

After:
```cpp
bool f(int x) {
    bool b = x & 0x02;
    return b;
}
```

## Related checkers

- [mismatchingBitAnd.md](mismatchingBitAnd.md) - a different bitmask mistake: chained `&` masks that
  share no bits.
- [comparisonError.md](comparisonError.md) - a bitwise expression compared against a constant it can
  never produce.
