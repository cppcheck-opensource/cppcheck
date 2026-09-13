# mallocOnClassError and mallocOnClassWarning

**Message**: Memory for class instance allocated with malloc(), but class contains a virtual function.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error/Warning<br/>
**Language**: C++

## Description

A C++ class/struct instance is allocated with `malloc()`/`calloc()`/`realloc()` instead of `new` - the
constructor never runs.

- `mallocOnClassError`: the class clearly needs its constructor to run (it has a virtual function, so
  its vtable pointer is never set up, or a non-trivial member type that itself needs construction).
- `mallocOnClassWarning`: a constructor exists but the class might still happen to work out by luck
  (for example if the constructor only sets members to values that also happen to match zeroed memory).

## Motivation

`malloc()` only allocates raw memory - it never calls a C++ constructor, so no object of that class type
actually exists in that memory yet. The `malloc()` call itself doesn't crash or misbehave; the risk is
in what happens next - calling any member function through the pointer (especially a virtual one, which
reads a vtable pointer that was never set up), or letting a destructor run on it, is undefined behaviour,
because the standard's rules for using an object of a type only apply once that object's lifetime has
actually begun via a constructor call.

## How to fix

Before:
```cpp
#include <cstdlib>
struct C { virtual void bar(); };
void foo(C*& p) {
    p = malloc(sizeof(C)); // <- mallocOnClassError: no constructor/vtable set up
}
```

After:
```cpp
struct C { virtual void bar(); };
void foo(C*& p) {
    p = new C();
}
```

Before:
```cpp
#include <cstdlib>
class C { public: C() {} };
void foo(C*& p) {
    p = malloc(sizeof(C)); // <- mallocOnClassWarning: C()'s body never runs
}
```

After:
```cpp
class C { public: C() {} };
void foo(C*& p) {
    p = new C();
}
```

