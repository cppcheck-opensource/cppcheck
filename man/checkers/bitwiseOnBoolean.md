# bitwiseOnBoolean

**Message**: Boolean expression 'x' is used in bitwise operation. Did you mean '&&'?<br/>
**Category**: Readability<br/>
**Severity**: Style (Inconclusive)<br/>
**Language**: C/C++ (also applies to C's `_Bool`)

## Description

`&`/`|`/`&=`/`|=` is used where at least one operand is boolean, when `&&`/`||` was likely intended.

## Motivation

This checker is about readability. Readability is subjective - opinions differ about what is more
readable. Please follow your own opinion.

When both operands are `bool`, `&`/`|` on their `0`/`1` representation happens to produce the same
truth value as `&&`/`||`, so this code usually still works correctly today. The main reason to flag it
anyway is common practice: `&&`/`||` is the conventional, unambiguous way to write boolean logic in
C/C++, while `&`/`|` is understood to mean bitwise work - so a stray single `&`/`|` reads as a likely
typo even when it happens to be harmless. There is also one real behavioural difference: `&`/`|` always
evaluates both operands, so if the other side has a side effect, using `&`/`|` instead of `&&`/`||`
changes whether that side effect happens.

When the *other* operand isn't itself boolean (e.g. an integer flag or count), `&`/`|` combines the
boolean's `0`/`1` value with it bit-by-bit, which generally is **not** the same truth value `&&`/`||`
would produce - in that case this points at an actual logic bug, not just a style preference.

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
