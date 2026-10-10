# nanInArithmeticExpression

**Message**: Using NaN/Inf in a computation.<br/>
**Category**: Correctness<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

A computation divides by `0.0` and immediately adds or subtracts another value from the result - the
addition/subtraction is pointless once the division has already produced `NaN`/`Inf`.

## Motivation

Once an expression is `NaN` or `Inf`, further arithmetic on it (other than a handful of operations
specifically meant to detect/handle that case) can't recover a meaningful value - so code that keeps
computing with it is either dead weight or a sign the `NaN`/`Inf` case wasn't actually intended.

## How to fix

Before:
```cpp
#include <cstdio>
void f() {
   double x = 3.0 / 0.0 + 1.0; // <- pointless once the division already produced NaN/Inf
   printf("%f", x);
}
```

After:
```cpp
#include <cstdio>
void f(double divisor) {
   double x = 3.0 / divisor + 1.0;
   printf("%f", x);
}
```

## Related checkers

- [zerodiv.md](zerodiv.md) - integer division by zero, which is undefined behaviour rather than the
  well-defined `NaN`/`Inf` result of floating-point division by zero.
