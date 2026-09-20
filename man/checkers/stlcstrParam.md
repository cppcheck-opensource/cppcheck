# stlcstrParam

**Message**: Passing the result of c_str() to a function that takes std::string as argument no. 1 is slow and redundant.<br/>
**Category**: Performance<br/>
**Severity**: Performance<br/>
**Language**: C++

## Description

`.c_str()` is used purely to feed a `const char*` into a function parameter that's typed `std::string`
- going through `c_str()` forces an unnecessary `strlen()`/copy that passing the `std::string` directly
would avoid.

## Motivation

Calling `.c_str()` just to hand the result to a parameter that would have happily accepted the
`std::string` itself throws away the length information the `std::string` already had, forcing a
`strlen()` scan (and a copy) to reconstruct it. Passing the `std::string` directly is both simpler and
faster.

## How to fix

Before:
```cpp
#include <string>
void Foo1(const std::string& s);
void f() {
    std::string str = "bar";
    Foo1(str.c_str()); // <- Foo1() already accepts a std::string
}
```

After:
```cpp
#include <string>
void Foo1(const std::string& s);
void f() {
    std::string str = "bar";
    Foo1(str);
}
```

## Related checkers

- [stlcstr.md](stlcstr.md) - the dangerous (not just inefficient) version of this mistake, where the
  pointer ends up dangling.
- [stlcstrReturn.md](stlcstrReturn.md), [stlcstrConstructor.md](stlcstrConstructor.md),
  [stlcstrAssignment.md](stlcstrAssignment.md), [stlcstrConcat.md](stlcstrConcat.md),
  [stlcstrStream.md](stlcstrStream.md) - the same idea in other call-site shapes.
