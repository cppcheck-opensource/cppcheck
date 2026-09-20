# operatorEqToSelf

**Message**: 'operator=' should check for assignment to self to avoid problems with dynamic memory.<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C++

## Description

`operator=` allocates/frees a resource but never checks for `a = a;` (self-assignment) - freeing a
resource and then trying to copy from the same, now-freed object corrupts the object.

## Motivation

`a = a;` is valid, if unusual, code - and an `operator=` that frees its own current resource before
copying from the source object can, on self-assignment, free the very resource it's about to read from,
leaving the object corrupted (a use of freed memory, which is undefined behaviour) - though this depends
on the new value actually being derived from the freed data; cppcheck flags any allocate-without-a-
self-check pattern in `operator=`, not just the ones it can confirm would read something already freed.
Since self-assignment is rare in normal code (it usually happens indirectly, through an alias or a
container operation), this bug can go unnoticed for a long time.

## How to fix

Check for self-assignment (`this == &a`) before freeing anything, and return early if it's true.

Before:
```cpp
#include <cstring>
#include <cstdlib>
class A {
public:
    char *s;
    A & operator=(const A &a)
    {
        free(s); // <- breaks if 'a' is '*this'
        s = strdup(a.s);
        return *this;
    }
};
```

After:
```cpp
#include <cstring>
#include <cstdlib>
class A {
public:
    char *s;
    A & operator=(const A &a)
    {
        if (this == &a)
            return *this;
        free(s);
        s = strdup(a.s);
        return *this;
    }
};
```
