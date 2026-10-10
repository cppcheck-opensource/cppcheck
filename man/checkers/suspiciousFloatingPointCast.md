# suspiciousFloatingPointCast

**Message**: Floating-point cast causes loss of precision.<br/>
**Category**: Correctness<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

A `double`/`long double` value is cast down to a narrower floating type (`float`, or `double` from
`long double`) and then used somewhere that expected the original, wider type back - the narrowing
silently throws away precision for no benefit.

## Motivation

Casting to a narrower floating type and then immediately using the result as the wider type again
gains nothing - the precision lost in the cast can't come back, and the code would behave identically
(with better precision) if the cast were simply removed.

## How to fix

Before:
```cpp
double f(double a, double b, float c) {
    return a + (float)b + c; // <- 'b' loses precision for no reason
}
```

After:
```cpp
double f(double a, double b, float c) {
    return a + b + c;
}
```
