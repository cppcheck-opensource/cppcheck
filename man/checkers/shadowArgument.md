# shadowArgument

**Message**: Local variable 'x' shadows outer argument<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C++

## Description

A local variable has the same name as one of the function's own parameters, which is still in scope -
the local one hides the parameter for the rest of its scope.

## Motivation

A shadowed name invites confusion about which variable a later line actually refers to, and is a
standing invitation to edit the wrong one.

## How to fix

Before:
```cpp
#include <cstdio>
void f(int x) { { int x = 1; printf("%d", x); } } // <- hides the parameter 'x'
```

After:
```cpp
#include <cstdio>
void f(int x) { { int y = 1; printf("%d", y); } }
```

## Related checkers

- [shadowVariable.md](shadowVariable.md) - the same idea, when the outer name is a variable.
- [shadowFunction.md](shadowFunction.md) - the same idea, when the outer name is a function.
- [shadowMember.md](shadowMember.md) - the same idea, when the outer name is a class/struct member.
