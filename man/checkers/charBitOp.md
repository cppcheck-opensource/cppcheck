# charBitOp

**Message**: When using 'char' variables in bit operations, sign extension can generate unexpected results.<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

A `char` variable is used as an operand of `&`/`|`/`^` and the result is stored in a wider type
(`short`/`int`/`long`) - if the `char` is negative, sign extension fills the extra bits with `1`s
instead of `0`s, so the resulting bit pattern isn't the one that was probably intended.

## Motivation

On many platforms `char` can hold negative values (typically -128..127 instead of 0..255). When a
negative `char` is used in a bitwise operation with a wider type, it's first sign-extended - the extra,
high-order bits of the wider type are filled with copies of the sign bit (`1` for a negative value)
rather than `0`. Code that expects a `char`'s bit pattern to occupy only its low 8 bits and leave the
rest zero gets a surprising result whenever the `char` happens to be negative.

## How to fix

Use `unsigned char` (or mask the operand) if the intent is to treat the byte as a small non-negative
number.

Before:
```cpp
void foo(int a, int *result) {
    signed char ch = -1;
    *result = a | ch; // <- sign extension fills the high bits of 'ch' with 1s
}
```

After:
```cpp
void foo(int a, int *result) {
    unsigned char ch = 0xff;
    *result = a | ch;
}
```

## Related checkers

- [signedCharArrayIndex.md](signedCharArrayIndex.md) - the same signed-`char` sign-extension pitfall,
  but for using a `char` as an array index instead of a bitwise operand.
- [checkCastIntToCharAndBack.md](checkCastIntToCharAndBack.md) - a different `char`-narrowing pitfall,
  about storing `getchar()`'s return value in a `char` before comparing it with `EOF`.
