# useInitializationList

**Message**: Variable 's' is assigned in constructor body. Consider performing initialization in initialization list.<br/>
**Category**: Performance<br/>
**Severity**: Performance<br/>
**Language**: C++

## Description

A constructor assigns a member a value in its body, where the exact same value could have been passed
via the member-initializer list instead - doing it in the list avoids first default-constructing the
member and then immediately overwriting it.

## Motivation

Assigning a member in the constructor body means the member is first default-constructed and then
immediately reassigned - for a type with a nontrivial default constructor (like `std::string`), that's
extra, avoidable work done on every single object construction.

## How to fix

Move the assignment into the member-initializer list.

Before:
```cpp
#include <string>
class C {
    std::string s;
public:
    explicit C(const std::string& str) {
        s = str; // <- could be done in the initializer list instead
    }
};
```

After:
```cpp
#include <string>
class C {
    std::string s;
public:
    explicit C(const std::string& str) : s(str) {}
};
```

## Related checkers

- [initializerList.md](initializerList.md) - a different constructor-initializer-list pitfall, about
  the *order* members are listed in, rather than whether the list is used at all.
