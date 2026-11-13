# useStandardLibrary

**Message**: Consider using std::memcpy instead of loop.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C++ only

## Description

A hand-written byte-by-byte copy or fill loop that could be replaced with `memcpy`/`memset`.

## Motivation

A standard library function like `memcpy()` is at least as fast as a hand-written loop (the library or
compiler can vectorize/optimize it directly), is shorter to read, and doesn't need to be double-checked
for off-by-one mistakes the way a raw loop does.

## How to fix

Before:
```cpp
#include <cstdint>
void f(void* dst, const void* src, const size_t count) {
    size_t i;
    for (i = 0; count > i; ++i) // <- hand-written copy loop
        (reinterpret_cast<uint8_t*>(dst))[i] = (reinterpret_cast<const uint8_t*>(src))[i];
}
```

After:
```cpp
#include <cstring>
void f(void* dst, const void* src, const size_t count) {
    std::memcpy(dst, src, count);
}
```

## Related checkers

- [returnStdMoveLocal.md](returnStdMoveLocal.md) - an unrelated performance suggestion in the same
  checker: avoiding a `std::move()` that defeats copy elision.
