# stlcstrReturn

**Message**: Returning the result of c_str() in a function that returns std::string is slow and redundant.<br/>
**Category**: Performance<br/>
**Severity**: Performance<br/>
**Language**: C++

## Description

`.c_str()` is used purely to feed a `const char*` into a `return` statement of a function that returns
`std::string` - going through `c_str()` forces an unnecessary `strlen()`/copy that constructing the
`std::string` result directly would avoid.

## Motivation

Calling `.c_str()` just to hand the result back to something that would have happily accepted the
`std::string` itself throws away the length information the `std::string` already had, forcing a
`strlen()` scan (and a copy) to reconstruct it. Passing/returning the `std::string` directly is both
simpler and faster.

## How to fix

Before:
```cpp
#include <string>
std::string get_msg() {
    std::string errmsg;
    return errmsg.c_str(); // <- forces an unnecessary strlen()/copy
}
```

After:
```cpp
#include <string>
std::string get_msg() {
    std::string errmsg;
    return errmsg;
}
```

## Related checkers

- [stlcstr.md](stlcstr.md) - the dangerous (not just inefficient) version of this mistake, where the
  function returns `const char*` and the pointer ends up dangling.
- [stlcstrParam.md](stlcstrParam.md), [stlcstrConstructor.md](stlcstrConstructor.md),
  [stlcstrAssignment.md](stlcstrAssignment.md), [stlcstrConcat.md](stlcstrConcat.md),
  [stlcstrStream.md](stlcstrStream.md) - the same idea in other call-site shapes.
