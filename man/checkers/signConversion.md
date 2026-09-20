# signConversion

**Message**: Expression 'x' can have a negative value. That is converted to an unsigned value and used in an unsigned calculation.<br/>
**Category**: Type Safety<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

This checker detects arithmetic expressions (other than `+`/`-`) whose result type is unsigned, where
one of the operands can have a negative value. When that happens, the negative operand is implicitly
converted to an unsigned value before the calculation, which can produce a very large value instead
of the intended negative one.

If the negative value is a known constant, the message states the operand "has" a negative value;
otherwise it states the operand "can have" a negative value, based on cppcheck's analysis of the
surrounding code.

This checker only runs when the `warning` severity is enabled.

## Motivation

Implicit conversion of a negative value to an unsigned type is well-defined (it wraps around to a
large positive value), but it rarely matches programmer intent and is a common source of bugs, for
example in loop conditions, index calculations and buffer size calculations.

## Design note: `+` and `-` are intentionally not checked

This checker does not warn about `+` and `-`, even though the same implicit negative-to-unsigned
conversion happens there too. This is intentional, not an oversight: for `+` and `-`, adding an
explicit cast would not change the computed result at all.

```cpp
void f(int x) { // x can be negative
    unsigned int a = x + 5U;          // implicit conversion
    unsigned int b = (unsigned int)x + 5U; // explicit cast - identical result
}
```

Two's-complement addition and subtraction commute with truncation to an unsigned width, so the
implicit conversion already computes exactly what an explicit `(unsigned int)` cast would. There is
nothing for an explicit cast to fix or clarify, so a warning here would not be actionable. Other
arithmetic operators (`*`, `/`, `%`, shifts, etc.) do not have this property in the same way and are
still checked.

## How to fix

You can fix these warnings by:
1. Making sure the operand cannot be negative in that context (fix upstream logic)
2. Using a signed type for the calculation
3. Adding an explicit check or cast to make the intended behaviour clear

Note: cppcheck only warns when it can actually determine that the operand can be negative - either
from a known value at the call site (as below), or from a condition earlier in the same function. A
plain `int` parameter with no callers and no surrounding condition gives cppcheck no evidence that it
can be negative, so it is not reported.

Before:
```cpp
unsigned int calcSize(int count, unsigned int itemSize) {
    return count * itemSize; // <- signConversion, count can have a negative value
}

void caller() {
    calcSize(-4, 4);
}
```

After:
```cpp
unsigned int calcSize(int count, unsigned int itemSize) {
    if (count < 0)
        return 0;
    return count * itemSize;
}

void caller() {
    calcSize(-4, 4);
}
```
