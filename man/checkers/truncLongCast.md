# truncLongCastAssignment and truncLongCastReturn

**Message**: int result is assigned to long variable. If the variable is long to avoid loss of information, then you have loss of information.<br/>
**Category**: Type Safety<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

A multiplication (`*`) or left-shift (`<<`) is computed using `int` arithmetic, and only afterwards -
once any overflow has already happened - is the result widened to a larger integer type by an implicit
or explicit conversion. Widening after the fact doesn't recover information that `int` arithmetic already
lost.

## Motivation

Declaring a wider result type (`long`, `int64_t`, ...) is often meant to give a calculation more room, so
it doesn't overflow. That only works if the wider type is used for the calculation itself; if the
multiplication or shift is still done in `int` and only the final result is widened, the overflow already
happened before the conversion, and the wider type just carries forward a truncated value.

## How to fix

You can fix these warnings by:
1. Add explicit cast to avoid loss of information
2. Explicitly truncate the result
3. Change type of assigned variable

Before:
```cpp
void foo(int32_t y) {
    int64_t x = y * y; // <- warning
}
```

After (explicit cast):
```cpp
void foo(int32_t y) {
    int64_t x = (int64_t)y * y; // <- 64-bit multiplication
}
```

After (explicitly truncate the result):
```cpp
void foo(int32_t y) {
    int64_t x = (int32_t)(y * y); // redundant cast makes it explicit that your intention is to truncate the result to 32-bit
}
```

After (change type of assigned variable):
```cpp
void foo(int32_t y) {
    int32_t x = y * y;
}
```
