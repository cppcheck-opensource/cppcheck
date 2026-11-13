# memsetFloat and memsetValueOutOfRange

**Message**: The 2nd memset() argument 'x' is a float, its representation is implementation defined.<br/>
**Category**: Correctness<br/>
**Severity**: Portability/Warning<br/>
**Language**: C/C++

## Description

`memset()`'s fill value (2nd argument):

- `memsetFloat`: is a `float`, whose byte representation is implementation-defined - the actual bytes
  written will vary across platforms.
- `memsetValueOutOfRange`: is a literal integer that doesn't fit in an `unsigned char` - `memset()`
  only ever writes the `unsigned char` conversion of this value, so anything outside `0..255` (or the
  platform's signed-char range) doesn't mean what it looks like.

## Motivation

`memset()` fills memory byte-by-byte using the value converted to `unsigned char`. A `float` argument
has no portable byte-for-byte meaning in that context, and an integer literal outside a single byte's
range is silently truncated - both make the call's actual effect different from what the source code
suggests.

## How to fix

Before:
```cpp
void f(void* p, size_t n) {
    memset(p, 1.0f, n); // <- float, implementation-defined byte pattern
}
```

After:
```cpp
void f(void* p, size_t n) {
    memset(p, 0, n);
}
```

Before:
```cpp
void f(void* p, size_t n) {
    memset(p, 300, n); // <- 300 doesn't fit in an unsigned char
}
```

After:
```cpp
void f(void* p, size_t n) {
    memset(p, 0xff, n);
}
```

## Related checkers

- [memsetZeroBytes.md](memsetZeroBytes.md) - a different `memset()` misuse, about the length (3rd)
  argument being zero rather than the fill value.
