# virtualCallInConstructor and pureVirtualCall

**Message**: Call of pure virtual function 'pure' in constructor.<br/>
**Category**: Correctness/Undefined Behaviour<br/>
**Severity**: Warning/Error<br/>
**Language**: C++

## Description

A constructor or destructor calls a virtual function on `this`. During construction/destruction the
object's dynamic type is only ever the class currently running, so this never dispatches to a derived
override the way it looks like it should.

- `virtualCallInConstructor`: the called function does have a body in the current class, so the call is
  well-defined - it's just probably not calling what the author expected.
- `pureVirtualCall`: the called function has no implementation at all (pure virtual), so the call is
  undefined behaviour outright.

## Motivation

Code that calls a virtual function from a constructor or destructor, expecting a derived class's
override to run, is a common misunderstanding of C++'s object-construction model. `pureVirtualCall` in
particular is a crash waiting to happen, since there is no function body to call at all at that point.

## How to fix

Before:
```cpp
class A {
    virtual int f() { return 1; }
public:
    A();
};
A::A() {
    f(); // <- virtualCallInConstructor: dynamic binding doesn't apply here
}
```

After:
```cpp
class A {
    virtual int f() { return 1; }
    int init() { return 1; }
public:
    A();
};
A::A() {
    init();
}
```

Before:
```cpp
class A {
    virtual void pure() = 0;
public:
    A();
};
A::A() {
    pure(); // <- pureVirtualCall: 'pure' has no body to call
}
```

After:
```cpp
class A {
    virtual void pure() = 0;
public:
    A();
};
A::A() {
}
```

