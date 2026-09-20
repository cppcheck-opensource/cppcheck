# memsetClass

**Message**: Using 'memset' on class that contains a 'std::string'.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C++

## Description

`memset()`/`memcpy()`/`memmove()` is used on an object of a type that isn't POD (for example it
contains a `std::string`) - the constructor, destructor, and copy semantics that type relies on are all
bypassed.

## Motivation

Raw memory functions like `memset()` don't know anything about C++ object semantics - they just
overwrite bytes. Using one on an object that has its own constructor/destructor/internal invariants
(like `std::string`'s internal pointer/length bookkeeping) corrupts that object instead of resetting it.

## How to fix

Before:
```cpp
#include <cstring>
#include <string>
class Fred {
public:
    std::string b;
};
void f() {
    Fred fred;
    memset(&fred, 0, sizeof(Fred)); // <- memsetClass: bypasses std::string's own management
}
```

After:
```cpp
#include <string>
class Fred {
public:
    std::string b;
};
void f() {
    Fred fred;
}
```

## Related checkers

- [memsetClassFloat.md](memsetClassFloat.md) - the same underlying mistake, specifically for a type
  containing a `float`/`double` member.
- [memsetClassReference.md](memsetClassReference.md) - the same underlying mistake, specifically for a
  type containing a reference member.
