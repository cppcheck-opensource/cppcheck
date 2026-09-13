# integerOverflow and integerOverflowCond

**Message**: Signed integer overflow for expression 'x*y'.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error/Warning<br/>
**Language**: C/C++

## Description

This checker detects when a signed integer arithmetic expression (`+`, `-`, `*`, `/`, `<<`, etc.) can
overflow or underflow the range of its result type, based on the platform's configured integer
widths.

- `integerOverflow`: the overflow/underflow is unconditional - it happens for every value cppcheck can
  see reaching that point.
- `integerOverflowCond`: the overflow/underflow only happens under a certain condition elsewhere in the
  code; the message explains that "Either the condition ... is redundant or there is signed integer
  overflow/underflow ...".

As a special case, left-shifting into the sign bit (for example `1 << 31` for a 32-bit int) is not
reported, since this is common practice even though it is technically undefined behaviour; that is
instead covered by the [shiftTooManyBits](shiftTooManyBits.md) checker family.

## Motivation

Signed integer overflow is undefined behaviour in C and C++. In practice this often means the
calculation silently produces a wrong (wrapped or truncated) result, and with optimizations enabled
the compiler is allowed to assume overflow never happens, which can eliminate or reorder code in
surprising ways.

## How to fix

You can fix these warnings by:
1. Using a wider integer type for the calculation
2. Rewriting the calculation to avoid the overflow (for example checking bounds before multiplying)
3. Using an unsigned type, if wraparound behaviour is actually intended

Note: cppcheck only warns when it can actually determine that the calculation overflows - either from
a known value (as below) or from a condition elsewhere in the code (see `integerOverflowCond` above).
A plain `a * b` of two otherwise-unconstrained parameters gives cppcheck nothing to prove an overflow
with, so it is not reported.

Before:
```cpp
int32_t f() {
    int32_t intmax = 0x7fffffff; // INT32_MAX
    return intmax + 1; // <- integerOverflow, known to overflow 32-bit int
}
```

After:
```cpp
int64_t f() {
    int32_t intmax = 0x7fffffff;
    return (int64_t)intmax + 1; // <- widen before adding
}
```
