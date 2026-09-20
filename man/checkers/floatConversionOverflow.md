# floatConversionOverflow

**Message**: Undefined behaviour: float (1e+100) to integer conversion overflow.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error/Warning<br/>
**Language**: C/C++

## Description

This checker detects conversions from a floating point value to an integer type (via explicit cast,
assignment or `return`) where the floating point value is outside the range that the target integer
type can represent, based on the platform's configured integer widths.

## Motivation

Converting a floating point value to an integer type is undefined behaviour when the value does not
fit in the target type (for example, it is too large, too small, NaN, or infinite). Unlike integer
overflow, this cannot simply be assumed to "wrap around"; the actual result is unpredictable.

## How to fix

Make sure the floating point value fits in the target integer type before converting, for example by
clamping the value to the valid range, or by using a wider or floating point type instead.

Note: cppcheck only warns when it can determine the actual floating point value (or a bound on it).
An unconstrained `double` parameter with no known or derivable value is not checked; the example
below uses a value cppcheck can trace so the warning actually fires.

Before:
```cpp
int32_t foo() {
    double d = 1E100;
    return (int32_t)d; // <- floatConversionOverflow, 1E100 does not fit in a 32-bit int
}
```

After:
```cpp
int32_t foo() {
    double d = 1E100;
    if (d > (double)INT32_MAX || d < (double)INT32_MIN)
        return 0; // handle out-of-range value
    return (int32_t)d;
}
```
