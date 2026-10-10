# stlcstrConcat

**Message**: Concatenating the result of c_str() and a std::string is slow and redundant.<br/>
**Category**: Performance<br/>
**Severity**: Performance<br/>
**Language**: C++

## Description

`.c_str()` called on a `std::string` is concatenated (`+`) with another `std::string` - going through
`c_str()` forces an unnecessary `strlen()` call that concatenating the two `std::string`s directly
would avoid.

## Motivation

Concatenating a `const char*` with a `std::string` requires calling `strlen()` on the `const char*` to
find its length, even though the `std::string` it came from already knew its own length. Concatenating
the two `std::string`s directly skips that redundant scan.

## How to fix

Before:
```cpp
#include <string>
std::string g(const std::string& a, const std::string& b) {
    return a + b.c_str(); // <- forces a strlen() that's already known
}
```

After:
```cpp
#include <string>
std::string g(const std::string& a, const std::string& b) {
    return a + b;
}
```

## Related checkers

- [stlcstr.md](stlcstr.md) - the dangerous (not just inefficient) version of this mistake.
- [stlcstrReturn.md](stlcstrReturn.md), [stlcstrParam.md](stlcstrParam.md),
  [stlcstrConstructor.md](stlcstrConstructor.md), [stlcstrAssignment.md](stlcstrAssignment.md),
  [stlcstrStream.md](stlcstrStream.md) - the same idea in other call-site shapes.
