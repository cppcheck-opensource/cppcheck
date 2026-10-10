# unpreciseMathCall

**Message**: Expression 'exp(x) - 1' can be replaced by 'expm1(x)' to avoid loss of precision.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

An expression like `exp(x) - 1` or `log(1 + x)` should be written as `expm1(x)`/`log1p(x)` to avoid
losing precision for small `x`.

## Motivation

For small `x`, `exp(x)` is very close to `1`, so `exp(x) - 1` subtracts two nearly-equal
floating-point numbers - a classic way to lose most of the significant digits of the result.
`expm1()`/`log1p()` are designed to compute the same mathematical result without this cancellation.

## How to fix

Before:
```cpp
#include <cmath>
void f() {
    print(exp(3.5) - 1); // <- loses precision for small arguments
}
```

After:
```cpp
#include <cmath>
void f() {
    print(expm1(3.5));
}
```

## Related checkers

- [wrongmathcall.md](wrongmathcall.md) - a related but distinct math-function issue: passing a literal
  value outside a function's valid domain.
