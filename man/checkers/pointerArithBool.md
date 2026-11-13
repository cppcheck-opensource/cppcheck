# pointerArithBool

**Message**: Converting pointer arithmetic result to bool. The bool is always true unless there is undefined behaviour.<br/>
**Category**: Correctness<br/>
**Severity**: Error<br/>
**Language**: C++

## Description

The result of pointer arithmetic (`p + 1`, `p - 1`, ...) is used directly as a boolean condition.

## Motivation

A pointer produced by arithmetic is essentially always non-null (unless the arithmetic itself is
undefined behaviour), so converting it straight to `bool` is always `true` and rarely what the code's
author meant - usually a dereference was intended instead.

## How to fix

Before:
```cpp
void f(char *p) {
    if (p + 1) {} // <- always true unless undefined behaviour
}
```

After:
```cpp
void f(char *p) {
    if (p && *(p + 1)) {}
}
```

