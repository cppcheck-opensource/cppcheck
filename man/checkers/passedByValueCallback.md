# passedByValueCallback

**Message**: Parameter 'x' should be passed by const reference. However it seems that 'f' is a callback function.<br/>
**Category**: Code Quality<br/>
**Severity**: Performance<br/>
**Language**: C++

## Description

Same idea as [passedByValue.md](passedByValue.md): a non-trivial parameter is passed by value and never
modified, so it's copied for nothing. This variant is for when the function is used as a callback -
fixing it may also require adjusting the function pointer type it's assigned to.

## Motivation

Passing a non-trivial object by value makes an unnecessary copy every time the function is called. The
callback case is called out separately because the fix isn't purely local: the function pointer type
also needs to change.

## How to fix

Before:
```cpp
#include <string>
#include <cstdio>
void setCb(void (*cb)(std::string));
void cb(std::string s) { printf("%s", s.c_str()); } // <- an unnecessary copy of 's' is made
void f() { setCb(cb); }
```

After:
```cpp
#include <string>
#include <cstdio>
void setCb(void (*cb)(const std::string&));
void cb(const std::string& s) { printf("%s", s.c_str()); }
void f() { setCb(cb); }
```

## Related checkers

- [passedByValue.md](passedByValue.md) - the same idea, for a parameter that isn't part of a callback
  function's signature.
