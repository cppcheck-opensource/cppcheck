# memsetClassReference

**Message**: Using 'memset' on class that contains a reference.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C++

## Description

`memset()`/`memcpy()`/`memmove()` is used on a type containing a reference member - a reference can't
be reseated after it's bound, so overwriting its bytes doesn't do what an assignment would.

## Motivation

A reference member is bound once, at construction, and can never be made to refer to something else.
Overwriting its bytes with `memset()` doesn't rebind it - it corrupts whatever internal representation
the compiler uses for references, which is undefined behaviour.

## How to fix

Before:
```cpp
#include <cstring>
#include <string>
class A {
public:
  std::string &s;
  A(std::string &str) : s(str) {}
};
void f(std::string &str) {
  A a(str);
  memset(&a, 0, sizeof(a)); // <- memsetClassReference: can't overwrite a bound reference like this
}
```

After:
```cpp
#include <string>
class A {
public:
  std::string &s;
  A(std::string &str) : s(str) {}
};
void f(std::string &str) {
  A a(str);
}
```

## Related checkers

- [memsetClass.md](memsetClass.md) - the same underlying mistake, specifically for a type that isn't
  POD at all (for example it contains a `std::string`).
- [memsetClassFloat.md](memsetClassFloat.md) - the same underlying mistake, specifically for a type
  containing a `float`/`double` member.
