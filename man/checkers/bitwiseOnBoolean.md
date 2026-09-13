# bitwiseOnBoolean

**Message**: Boolean expression 'x' is used in bitwise operation. Did you mean '&&'?<br/>
**Category**: Code Quality<br/>
**Severity**: Style (Inconclusive)<br/>
**Language**: C/C++ (also applies to C's `_Bool`)

## Description

`&`/`|`/`&=`/`|=` is used where at least one operand is boolean, when `&&`/`||` was likely intended.

## Motivation

`&`/`|` and `&&`/`||` look similar but behave very differently: the bitwise operators always evaluate
both sides (no short-circuiting) and, for non-`bool` operands, work bit-by-bit rather than on the
boolean truth value - so a stray single `&`/`|` where `&&`/`||` was meant can silently change behaviour.

## How to fix

Before:
```cpp
void f(bool a, bool b) {
    if (a & b) {} // <- likely meant '&&'
}
```

After:
```cpp
void f(bool a, bool b) {
    if (a && b) {}
}
```
