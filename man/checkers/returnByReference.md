# returnByReference

**Message**: Function returns a copy instead of returning by const reference.<br/>
**Category**: Performance<br/>
**Severity**: Performance<br/>
**Language**: C++

## Description

A getter function returns a large member (or a container/string) by value, making an unnecessary copy
on every call, when it could return a `const` reference instead.

## Motivation

Returning a container or string by value copies its entire contents on every call, even though the
caller almost always just wants to look at the member that's already sitting inside the object.
Returning a `const&` instead avoids that copy entirely.

## How to fix

Before:
```cpp
#include <string>
struct S {
    std::string s;
    std::string getS() const { return s; } // <- returnByReference: copies 's' on every call
};
```

After:
```cpp
#include <string>
struct S {
    std::string s;
    const std::string& getS() const { return s; }
};
```
