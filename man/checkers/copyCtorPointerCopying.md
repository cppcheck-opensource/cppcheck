# copyCtorPointerCopying

**Message**: Value of pointer 'p', which points to allocated memory, is copied in copy constructor instead of allocating new memory.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Warning<br/>
**Language**: C++

## Description

A copy constructor copies a pointer member's *value* (so both objects now point at the same allocated
memory) instead of allocating a fresh block and copying the pointed-to data - a classic shallow-copy
bug, usually followed by a double-free once both objects are destroyed.

## Motivation

Once two objects share the same pointer value for a member each believes it owns, whichever is
destroyed first frees the memory out from under the other - leaving the survivor with a dangling
pointer, and destroying both eventually frees the same block twice.

## How to fix

Allocate a fresh block in the copy constructor and copy the pointed-to data into it, rather than copying
the pointer itself.

Before:
```cpp
#include <cstring>
#include <cstdlib>
class F {
   char *p;
   F(const F &f) {
      p = f.p; // <- both objects now share the same allocated block
   }
public:
   F(char *str) {
      p = malloc(strlen(str)+1);
   }
   ~F();
   F& operator=(const F&f);
};
```

After:
```cpp
#include <cstring>
#include <cstdlib>
class F {
   char *p;
public:
   F(const F &f) {
      p = (char*)malloc(strlen(f.p)+1);
      strcpy(p, f.p);
   }
   F(char *str) {
      p = (char*)malloc(strlen(str)+1);
      strcpy(p, str);
   }
   ~F();
   F& operator=(const F&f);
};
```

## Related checkers

- [noCopyConstructor.md](noCopyConstructor.md) - the related check for when a class allocates a
  resource but doesn't define a copy constructor at all.
