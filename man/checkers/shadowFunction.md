# shadowFunction

**Message**: Local variable 'x' shadows outer function<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C++

## Description

A local variable (or parameter) has the same name as a function that is still visible - the local one
hides the function for the rest of its scope.

## Motivation

A shadowed name invites confusion about which one a later line actually refers to, and is a standing
invitation to edit the wrong thing - or to be surprised that the function can no longer be called by
its plain name in that scope.

## How to fix

Before:
```cpp
#include <cstdio>
int getA();
void f() { int getA = 1; printf("%d", getA); } // <- hides the function 'getA'
```

After:
```cpp
#include <cstdio>
int getA();
void f() { int result = 1; printf("%d", result); }
```

## Related checkers

- [shadowVariable.md](shadowVariable.md) - the same idea, when the outer name is a variable.
- [shadowArgument.md](shadowArgument.md) - the same idea, when the outer name is a function parameter.
- [shadowMember.md](shadowMember.md) - the same idea, when the outer name is a class/struct member.
