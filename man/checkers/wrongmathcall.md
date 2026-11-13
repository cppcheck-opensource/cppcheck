# wrongmathcall

**Message**: Passing value -2 to log() leads to implementation-defined result.<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

A literal value outside a math function's valid domain is passed directly (for example a negative
number to `log()`, or `0` as the divisor to `fmod()`). This check only looks at a literal numeric value
written directly in the call - it does not evaluate variables or computed expressions, even when their
value is otherwise known to be out of range.

## Motivation

Math functions like `log()`, `sqrt()`, or `asin()` are only defined for part of the real numbers; calling
them with a value outside that domain is undefined or implementation-defined and typically produces
`NaN` or a platform-specific result rather than a clean error.

## How to fix

Before:
```cpp
#include <cmath>
void f() {
    double y = log(-2); // <- log() of a negative number
}
```

After:
```cpp
#include <cmath>
void f(double x) {
    double y = (x > 0) ? log(x) : 0.0;
}
```

## Related checkers

- [unpreciseMathCall.md](unpreciseMathCall.md) - a related but distinct math-function issue: a
  precision-losing way of writing a calculation that a more precise function already covers.
