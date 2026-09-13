# shiftTooManyBits and shiftTooManyBitsSigned

**Message**: Shifting 32-bit value by 40 bits is undefined behaviour<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error/Warning/Portability<br/>
**Language**: C/C++

## Description

This checker warns when a bitwise shift (`<<`, `>>`, `<<=`, `>>=`) shifts a value by a number of bits
that is greater than or equal to the width of the (promoted) left-hand side type.

There are two related warnings:
- `shiftTooManyBits`: the shift amount is greater than or equal to the bit width of the type. This is
  undefined behaviour according to the C/C++ standard.
- `shiftTooManyBitsSigned`: the left-hand side type is signed and the shift amount is exactly
  `bits - 1`. Shifting a signed type this far is undefined behaviour before C++14, and
  implementation-defined behaviour from C++14 onwards - in the C++14-and-later case this is reported
  as a `portability` message instead of a `warning`, since the behaviour is now defined, just not the
  same on every platform.

The number of bits of the left-hand side type is determined from the platform settings
(`int_bit`, `long_bit`, `long_long_bit`), so this checker requires a platform to be configured.

## Motivation

Shifting a value by more bits than its type contains is undefined behaviour. The result is
unpredictable and can vary between compilers, compiler versions and optimization settings.

cppcheck only warns when it can work out the shift amount directly from the code - a literal value, or
a condition earlier in the function that pins it down. If the amount can't be determined that way (for
example it's guarded by several combined conditions, or hidden inside a macro-like call), no warning is
given even though the shift could still be too large at runtime.

## How to fix

Make sure the shift amount is smaller than the bit width of the left-hand side type. This often means
casting the left-hand side to a wider type before shifting, or fixing a wrong shift amount.

Before:
```cpp
int32_t foo(int32_t x) {
    return x << 40; // <- shiftTooManyBits, 'int' is only 32 bits
}
```

After:
```cpp
int64_t foo(int32_t x) {
    return (int64_t)x << 40; // <- widen the operand before shifting
}
```
