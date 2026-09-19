# uninitdata

**Message**: Memory is allocated but not initialized: x<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C/C++

## Description

Like [uninitvar.md](uninitvar.md), but specifically for memory obtained from an allocation function
(`malloc`, etc.) - the block exists, but its contents haven't been written yet.

## Motivation

Memory returned by `malloc()` and similar functions is not zero-initialized - it holds whatever bytes
happened to already be there. Reading through it before writing to it is undefined behaviour, just like
reading an uninitialized plain variable, but easier to overlook since the allocation itself looks like
it "created" the value.

## How to fix

Before:
```cpp
#include <cstdio>
#include <cstdlib>
struct ABC { int a; int b; };
void f() {
    struct ABC *abc = (struct ABC*)malloc(sizeof(struct ABC));
    printf("%d", abc->a); // <- uninitdata
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
  allocated memory.
