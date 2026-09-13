# shadowMember

**Message**: Local variable 'x' shadows outer member<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C++

## Description

A local variable inside a member function has the same name as a class/struct member - the local one
hides the member for the rest of its scope.

## Motivation

A shadowed name invites confusion about which one a later line actually refers to, and is a standing
invitation to edit the local variable when the member was intended, or vice versa.

## How to fix

Before:
```cpp
#include <cstdio>
struct S {
    int i{};
    void f() { int i = 1; printf("%d", i); } // <- hides the member 'i'
};
```

After:
```cpp
#include <cstdio>
struct S {
    int i{};
    void f() { int localCount = 1; printf("%d", localCount); }
};
```

## Related checkers

- [shadowVariable.md](shadowVariable.md) - the same idea, when the outer name is a variable.
- [shadowArgument.md](shadowArgument.md) - the same idea, when the outer name is a function parameter.
- [shadowFunction.md](shadowFunction.md) - the same idea, when the outer name is a function.
