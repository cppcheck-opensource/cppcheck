# suspiciousCase

**Message**: Found suspicious case label in switch(). Operator '&&' probably doesn't work as intended.<br/>
**Category**: Correctness<br/>
**Severity**: Warning (Inconclusive)<br/>
**Language**: C/C++

## Description

A `switch` `case` label contains `&&`/`||` (`case A&&B:`) - a case label must be a single constant, so
this doesn't test both `A` and `B`; it's normally a sign that `if`/`else` or several `case` labels were
intended instead.

## Motivation

A `case` label is required to be a single constant expression - `A && B` is still just one expression
(evaluating to `0` or `1`), not a test of "when `A` and `B`." Code written this way almost never means
what a reader would guess from the `&&`/`||` at a glance, and usually indicates the author meant to test
multiple values or conditions in a way `switch` doesn't directly support.

## How to fix

Use separate `case` labels (to match several values) or an `if`/`else` chain (to test a real logical
condition) instead.

Before:
```cpp
void foo(int a, int A, int B) {
    switch(a) {
        case A&&B: // <- a case label must be one constant, not a logical expression
            foo(a, A, B);
    }
}
```

After:
```cpp
void foo(int a, int A, int B) {
    switch(a) {
        case A:
        case B:
            foo(a, A, B);
    }
}
```
