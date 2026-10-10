# stlcstr and stlcstrthrow

**Message**: Dangerous usage of c_str(). The value returned by c_str() is invalid after this call.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C++

## Description

The pointer from `.c_str()` (or an implicit conversion to `std::string` followed by `.c_str()`) escapes
the function it was taken in, in a way that leaves it dangling:

- `stlcstr`: the pointer is returned from a function - the string it points into was a temporary or
  local variable that no longer exists once the function returns.
- `stlcstrthrow`: the pointer is thrown as an exception - the string it points into is destroyed as the
  stack unwinds, so the caught pointer is immediately dangling.

## Motivation

`std::string::c_str()` only stays valid for as long as the `std::string` it came from is still alive
(and hasn't been modified). Returning or throwing that pointer instead of the string itself is a
dangling-pointer bug: the pointer looks fine at the point it's produced, and only misbehaves later,
wherever it's eventually used.

## How to fix

Before:
```cpp
#include <string>
const char *get_msg() {
    std::string errmsg;
    return errmsg.c_str(); // <- stlcstr: dangling as soon as 'errmsg' is destroyed
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

Before:
```cpp
#include <string>
void f() {
    std::string errmsg;
    throw errmsg.c_str(); // <- stlcstrthrow: dangling once the exception propagates past 'errmsg'
}
```

After:
```cpp
#include <string>
void f() {
    std::string errmsg;
    throw errmsg;
}
```

## Related checkers

- [stlcstrReturn.md](stlcstrReturn.md), [stlcstrParam.md](stlcstrParam.md),
  [stlcstrConstructor.md](stlcstrConstructor.md), [stlcstrAssignment.md](stlcstrAssignment.md),
  [stlcstrConcat.md](stlcstrConcat.md), [stlcstrStream.md](stlcstrStream.md) - the same
  `.c_str()`-when-a-`std::string`-would-do idea, in other call-site shapes; those are inefficient
  rather than dangerous.
