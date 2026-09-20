# shadowVariable

**Message**: Local variable 'x' shadows outer variable<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C++

## Description

A local variable (or parameter) has the same name as an outer variable that is still in scope - the
inner one hides the outer one for the rest of its scope.

## Motivation

A shadowed name invites confusion about which variable a later line actually refers to, and is a
standing invitation to edit the wrong one.

## How to fix

Before:
```cpp
#include <cstdio>
int x;
void f() { int x = 1; printf("%d", x); } // <- hides the global 'x'
```

After:
```cpp
#include <cstdio>
int x;
void f() { int y = 1; printf("%d", y); }
```

## Related checkers

- [shadowArgument.md](shadowArgument.md) - the same idea, when the outer name is a function parameter.
- [shadowFunction.md](shadowFunction.md) - the same idea, when the outer name is a function.
- [shadowMember.md](shadowMember.md) - the same idea, when the outer name is a class/struct member.
