# uninitStructMember

**Message**: Uninitialized struct member: x<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C/C++

## Description

A specific struct/class member is read before that particular member has been set, even if other
members of the same variable have been.

## Motivation

Initializing some members of a struct doesn't initialize the rest - reading a member that was never
assigned is undefined behaviour, the same as reading an uninitialized plain variable, and easy to miss
when other, nearby members of the same variable were set correctly.

## How to fix

Before:
```cpp
#include <cstdio>
#include <cstdlib>
struct ABC { int a; int b; };
void f() {
    struct ABC *abc = (struct ABC*)malloc(sizeof(struct ABC));
    printf("%d", abc->a); // <- uninitStructMember
}
```

After:
```cpp
#include <cstdio>
#include <cstdlib>
struct ABC { int a; int b; };
void f() {
    struct ABC *abc = (struct ABC*)malloc(sizeof(struct ABC));
    abc->a = 0;
    printf("%d", abc->a);
}
```

## Related checkers

- [uninitvar.md](uninitvar.md) - the general "read before assignment" check this one specializes for
  one particular struct member.
