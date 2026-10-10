# noDestructor

**Message**: Class 'F' does not have a destructor which is recommended since it has dynamic memory/resource management.<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C++

## Description

The class allocates a resource itself (`new`/`malloc`-family in a constructor) but doesn't define its
own destructor.

## Motivation

Without a destructor, whatever a constructor allocated is never released when the object is destroyed -
a straightforward memory/resource leak every time an object of this class goes out of scope or is
deleted.

## How to fix

Add a destructor that releases what the constructor allocated.

Before:
```cpp
struct F {
    char *p;
    F() { p = new char[10]; } // <- no destructor
    F(const F&);
    F& operator=(const F&);
};
```

After:
```cpp
struct F {
    char *p;
    F() { p = new char[10]; }
    F(const F&);
    F& operator=(const F&);
    ~F() { delete[] p; }
};
```

## Related checkers

- [noCopyConstructor.md](noCopyConstructor.md) and [noOperatorEq.md](noOperatorEq.md) - the same
  underlying "allocates a resource but is missing a special member function" pattern, for the copy
  constructor and `operator=` respectively.
