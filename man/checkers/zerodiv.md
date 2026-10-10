# zerodiv and zerodivcond

**Message**: Division by zero.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error/Warning<br/>
**Language**: C/C++

## Description

- `zerodiv`: an integer division or `%` has a divisor that's known to be zero.
- `zerodivcond`: the same problem, but the zero divisor only holds on one branch of a condition
  tested elsewhere - so either that condition is redundant, or this is a genuine division by zero.

## Motivation

Integer division by zero is undefined behaviour: the program's actual behaviour (a crash, a trap, or
something else) is not guaranteed by the language standard and can change with the compiler,
optimization level, or platform.

## How to fix

Before:
```cpp
#include <iostream>
void foo() {
    std::cout << 42 / (int)0; // <- zerodiv
}
```

After:
```cpp
#include <iostream>
void foo(int divisor) {
    std::cout << 42 / divisor;
}
```

Before:
```cpp
int f(int x, int y) {
    if (x == y) {}
    return 1 / (x-y); // <- zerodivcond: division by zero if x == y
}
```

After:
```cpp
int f(int x, int y) {
    if (x == y)
        return 0;
    return 1 / (x-y);
}
```

## Design notes

- **Dividing a floating-point value by `0.0` is not reported as `zerodiv`.** Unlike integer division,
  floating-point division by zero is well-defined by IEEE 754 (it produces `Inf` or `NaN` rather than
  undefined behaviour), so cppcheck only flags it separately, and only in the narrow shape described in
  [nanInArithmeticExpression.md](nanInArithmeticExpression.md).
