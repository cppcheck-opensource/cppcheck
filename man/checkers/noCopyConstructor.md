# noCopyConstructor

**Message**: Struct 'F' does not have a copy constructor which is recommended since it has dynamic memory/resource management.<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C++

## Description

The class allocates a resource itself (`new`/`malloc`-family in a constructor, `delete`/`free`-family
in a destructor) but doesn't define its own copy constructor (or only defaults it) - the
compiler-generated version would copy the raw pointer, not the resource.

## Motivation

The compiler-generated copy constructor just copies each member's value. For a raw pointer, that means
the copy ends up pointing at the exact same allocated block as the original - both objects now believe
they own it, so destroying either one leaves the other holding a dangling pointer, and destroying both
frees the same memory twice. Using the dangling copy afterward, or freeing the same block twice, is
undefined behaviour - though which of those actually happens, if either, depends on what the rest of the
program goes on to do with the two objects. This check fires on the class's shape alone (it allocates a
resource but has no copy constructor of its own) - it doesn't verify that the class is ever actually
copied anywhere in the analyzed code, so a class that's never copied is flagged just the same as one that
is.

## How to fix

Write a copy constructor that allocates a fresh block for the copy (or use a container/smart pointer
that already manages this correctly, avoiding the need for a hand-written one at all).

Before:
```cpp
class F {
    char *p;
public:
    F() { p = new char[10]; } // <- no copy constructor
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

- [noOperatorEq.md](noOperatorEq.md) and [noDestructor.md](noDestructor.md) - the same underlying
  "allocates a resource but is missing a special member function" pattern, for `operator=` and the
  destructor respectively.
- [copyCtorPointerCopying.md](copyCtorPointerCopying.md) - the more specific case where a copy
  constructor does exist, but shallow-copies a pointer instead of allocating a fresh block.
