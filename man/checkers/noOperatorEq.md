# noOperatorEq

**Message**: Struct 'F' does not have a operator= which is recommended since it has dynamic memory/resource management.<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C++

## Description

The class allocates a resource itself (`new`/`malloc`-family in a constructor, `delete`/`free`-family
in a destructor) but doesn't define its own `operator=` (or only defaults it) - the compiler-generated
version would copy the raw pointer, not the resource.

## Motivation

The compiler-generated assignment operator just copies each member's value. For a raw pointer, that
means the assigned-to object ends up pointing at the same allocated block as the source - both objects
now believe they own it, and the assigned-to object's original allocation is leaked in the process,
since nothing frees it first. This is more than a leak waiting to happen: if an object of this class is
ever assigned to another, both are eventually destroyed, and their destructors each free the same block,
that is a double-free - undefined behaviour. This check fires on the class's shape alone (it allocates a
resource but has no `operator=` of its own) - it doesn't verify that the class is ever actually assigned
anywhere in the analyzed code, so a class that's never copy-assigned is flagged just the same as one that
is.

## How to fix

Write an `operator=` that allocates a fresh block for the assigned-to object (or use a
container/smart pointer that already manages this correctly).

Before:
```cpp
class F {
    char *p;
public:
    F() { p = new char[10]; } // <- no operator=
    ~F() { delete[] p; }
};
```

After:
```cpp
class F {
    char *p;
public:
    F() { p = new char[10]; }
    F(const F& other) { p = new char[10]; }
    F& operator=(const F& other) { p = other.p; return *this; }
    ~F() { delete[] p; }
};
```

## Related checkers

- [noCopyConstructor.md](noCopyConstructor.md) and [noDestructor.md](noDestructor.md) - the same
  underlying "allocates a resource but is missing a special member function" pattern, for the copy
  constructor and destructor respectively.
