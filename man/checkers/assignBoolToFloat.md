# assignBoolToFloat

**Message**: Boolean value assigned to floating point variable.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C++

## Description

A `bool` value is assigned to a `float`/`double`/`long double` variable.

## Motivation

Assigning `true`/`false` to a floating-point variable relies on the implicit `bool`-to-float conversion
(`1.0`/`0.0`) and usually signals a typo or a variable of the wrong type, rather than an intentional
numeric `1.0`/`0.0`.

## How to fix

Before:
```cpp
void f() {
    double d = true; // <- likely meant a numeric value
}
```

After:
```cpp
void f() {
    double d = 1.0;
}
```
