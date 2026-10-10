# stlcstrStream

**Message**: Passing the result of c_str() to a stream is slow and redundant.<br/>
**Category**: Performance<br/>
**Severity**: Performance<br/>
**Language**: C++

## Description

`.c_str()` called on a `std::string` is streamed (`<<`) into an output stream - going through `c_str()`
forces an unnecessary `strlen()` call that streaming the `std::string` directly would avoid.

## Motivation

Streaming a `const char*` requires the stream to call `strlen()` on it to find its length, even though
the `std::string` it came from already knew its own length. Streaming the `std::string` directly skips
that redundant scan.

## How to fix

Before:
```cpp
#include <sstream>
#include <string>
void f(std::stringstream& strm, const std::string& s) {
    strm << s.c_str(); // <- forces a strlen() that's already known
}
```

After:
```cpp
#include <sstream>
#include <string>
void f(std::stringstream& strm, const std::string& s) {
    strm << s;
}
```

## Related checkers

- [stlcstr.md](stlcstr.md) - the dangerous (not just inefficient) version of this mistake.
- [stlcstrReturn.md](stlcstrReturn.md), [stlcstrParam.md](stlcstrParam.md),
  [stlcstrConstructor.md](stlcstrConstructor.md), [stlcstrAssignment.md](stlcstrAssignment.md),
  [stlcstrConcat.md](stlcstrConcat.md) - the same idea in other call-site shapes.
