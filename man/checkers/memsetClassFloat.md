# memsetClassFloat

**Message**: Using 'memset' on struct which contains a floating point number.<br/>
**Category**: Portability<br/>
**Severity**: Warning<br/>
**Language**: C++

## Description

`memset()`/`memcpy()`/`memmove()` is used on a type containing a `float`/`double` member - whether an
all-zero-bytes pattern actually means `0.0` on the target platform isn't something the C++ standard
guarantees.

## Motivation

Zeroing a struct's bytes to reset a floating-point member to `0.0` relies on the platform's
floating-point representation matching all-zero-bytes with the value zero. This is true in practice on
essentially all mainstream hardware (IEEE 754), but it's not guaranteed by the language, so it's a
portability risk rather than a certainty.

## How to fix

Before:
```cpp
#include <cstring>
typedef float realnum;
struct multilevel_data {
  realnum *GammaInv;
  realnum data[1];
};
void f() {
  multilevel_data d;
  memset(&d, 0, sizeof(multilevel_data)); // <- memsetClassFloat
}
```

After:
```cpp
typedef float realnum;
struct multilevel_data {
  realnum *GammaInv;
  realnum data[1];
};
void f() {
  multilevel_data d = {};
}
```

## Related checkers

- [memsetClass.md](memsetClass.md) - the same underlying mistake, specifically for a type that isn't
  POD at all (for example it contains a `std::string`).
- [memsetClassReference.md](memsetClassReference.md) - the same underlying mistake, specifically for a
  type containing a reference member.
