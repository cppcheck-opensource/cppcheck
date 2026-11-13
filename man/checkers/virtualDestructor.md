# virtualDestructor

**Message**: Class 'Base' which is inherited by class 'Derived' does not have a virtual destructor.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C++

## Description

A base class has virtual member functions (so it's clearly meant to be used polymorphically) but a
non-virtual destructor - deleting a derived object through a base-class pointer then only runs the base
class's destructor, leaking whatever the derived part owned.

## Motivation

This is outright undefined behaviour that can appear to "work" (the memory for the derived part is
still freed by the base's `delete`, even though its destructor never ran) until an unrelated change -
adding a derived class that owns a resource, for instance - makes it actually leak or corrupt memory.

## How to fix

Before:
```cpp
class Base {
public:
    virtual void f() {}
    ~Base() {} // <- virtualDestructor: not virtual
};
class Derived : public Base {
public:
    ~Derived() { (void)11; }
};
void f() {
    Base *base = new Derived;
    delete base;
}
```

After:
```cpp
class Base {
public:
    virtual void f() {}
    virtual ~Base() {}
};
class Derived : public Base {
public:
    ~Derived() override { (void)11; }
};
void f() {
    Base *base = new Derived;
    delete base;
}
```
