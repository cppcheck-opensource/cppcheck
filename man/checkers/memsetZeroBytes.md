# memsetZeroBytes

**Message**: memset() called to fill 0 bytes.<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

`memset(ptr, value, 0)` - the 2nd and 3rd arguments look swapped, since filling 0 bytes has no effect.

## Motivation

`memset()`'s signature is `memset(void *ptr, int value, size_t num)` - a literal `0` as the last
argument means the call does nothing at all, which is almost always a sign the fill value and the
length were written in the wrong order.

## How to fix

Before:
```cpp
void f(void* p) {
    memset(p, sizeof(p), 0); // <- value and length look swapped
}
```

After:
```cpp
void f(void* p, size_t n) {
    memset(p, 0, n);
}
```

## Related checkers

- [memsetFloat.md](memsetFloat.md) - other `memset()` misuses about the fill-value (2nd) argument
  rather than the length.
