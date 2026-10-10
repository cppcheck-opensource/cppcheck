# duplInheritedMember

**Message**: The class 'Derived' defines member variable with name 'x' also defined in its parent class 'Base'.<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C++

## Description

A derived class declares a data member or (non-virtual, non-constructor/destructor) member function
with the same name as one a base class already has - the derived one hides the base one, which is
confusing and easy to do by accident in a large class hierarchy.

## Motivation

When a derived class redeclares a name its base class already uses, the derived member hides the base
one for code that operates on the derived type - but code that only sees the base class (through a base
pointer/reference, or inside a base-class member function) still reaches the *base* member. The two
names look identical at each call site, so which one is actually being used depends entirely on the
static type in scope at that point, which is easy to get wrong.

## How to fix

Rename one of the two members so there's no ambiguity about which one a given piece of code refers to.

Before:
```cpp
class Base {
   protected:
   int x;
};
struct Derived : Base {
   int x; // <- hides 'Base::x'
};
```

After:
```cpp
class Base {
   protected:
   int x;
};
struct Derived : Base {
   int y;
};
```
