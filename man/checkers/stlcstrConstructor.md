# stlcstrConstructor

**Message**: Constructing a std::string from the result of c_str() is slow and redundant.<br/>
**Category**: Performance<br/>
**Severity**: Performance<br/>
**Language**: C++

## Description

A `std::string` is constructed from the result of `.c_str()` called on another `std::string` - going
through `c_str()` forces an unnecessary `strlen()` call that constructing directly from the source
`std::string` would avoid.

## Motivation

Constructing a `std::string` from a `const char*` requires calling `strlen()` to find its length, even
though the source `std::string` already knows its own length. Constructing from the `std::string`
directly skips that redundant scan.

## How to fix

Before:
```cpp
#include <string>
std::string f(const std::string& a) {
    std::string b(a.c_str()); // <- forces a strlen() that's already known
    return b;
}
```

After:
```cpp
#include <string>
std::string f(const std::string& a) {
    std::string b(a);
    return b;
}
```

## Related checkers

- [stlcstr.md](stlcstr.md) - the dangerous (not just inefficient) version of this mistake.
- [stlcstrReturn.md](stlcstrReturn.md), [stlcstrParam.md](stlcstrParam.md),
  [stlcstrAssignment.md](stlcstrAssignment.md), [stlcstrConcat.md](stlcstrConcat.md),
  [stlcstrStream.md](stlcstrStream.md) - the same idea in other call-site shapes.
