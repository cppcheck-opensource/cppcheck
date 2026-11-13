# uninitMemberVar, uninitMemberVarPrivate, uninitDerivedMemberVar, uninitDerivedMemberVarPrivate, uninitMemberVarNoCtor, uninitMemberVarPrivateNoCtor, uninitDerivedMemberVarNoCtor and uninitDerivedMemberVarPrivateNoCtor

**Message**: Member variable 'x' is not initialized in the constructor.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Warning<br/>
**Language**: C++

## Description

A constructor (or copy/move constructor) doesn't give a member variable any value. All eight IDs here
are the exact same underlying finding, distinguished only by three independent, combinable details of
the situation:

- **`Derived`** appears in the ID when the uninitialized member actually belongs to a *base* class
  rather than the class whose constructor is being looked at.
- **`Private`** appears in the ID when either there's no constructor at all and only some of the
  class's members happen to have in-class default values (so the ones without one are flagged this
  way), or the specific constructor that's missing the initialization is itself `private`.
- **`NoCtor`** appears in the ID when the class has no constructor at all, but *some* of its members do
  have in-class default initializers - which means cppcheck can tell that whoever wrote the class was
  thinking about initialization, making it worth flagging any other member that has no default.

Of the eight combinations, three (`uninitMemberVarPrivateNoCtor`, `uninitDerivedMemberVarNoCtor`,
`uninitDerivedMemberVarPrivateNoCtor`) are only theoretically possible - in the current version of
cppcheck, none of the code paths that would build these particular three ID strings are actually
reachable, so you will not see them in practice.

## Motivation

Reading a member before it's ever been given a value is undefined behaviour, whether or not the class
manages any resource - the member's contents are simply whatever bytes happened to already be in that
memory.

## How to fix

Give every member a value, either via an in-class default initializer or in every constructor's
member-initializer list.

Before:
```cpp
class Fred {
public:
    Fred() {} // <- 'i' is never given a value
    int i;
};
```

After:
```cpp
class Fred {
public:
    Fred() : i(0) {}
    int i;
};
```

Before:
```cpp
class C {
private:
    int i1 = 0;
    int i2; // <- no constructor, and 'i2' has no default like 'i1' does
};
```

After:
```cpp
class C {
private:
    int i1 = 0;
    int i2 = 0;
};
```

Before:
```cpp
class Base {
public:
    virtual void foo() = 0;
    int x; // <- left uninitialized by every class that derives from Base
};
class Derived: public Base {
public:
    Derived() {}
    void foo() override;
};
```

After:
```cpp
class Base {
public:
    Base() : x(0) {}
    virtual void foo() = 0;
    int x;
};
class Derived: public Base {
public:
    Derived() {}
    void foo() override;
};
```

Before:
```cpp
class B { int i; }; // <- B's own (private) constructor doesn't init 'i'
class D : B {
    explicit D(int) {}
};
```

After:
```cpp
class B {
    int i;
public:
    B() : i(0) {}
};
class D : B {
    explicit D(int) {}
};
```

Before:
```cpp
struct S {
    int a = 0, b; // <- 'a' has a default, 'b' doesn't
};
```

After:
```cpp
struct S {
    int a = 0, b = 0;
};
```

## Related checkers

- [noConstructor.md](noConstructor.md) - the related check for when a class has no constructor at all.
- [missingMemberCopy.md](missingMemberCopy.md) - the analogous finding for a copy/move constructor that
  simply forgets to copy one particular member, rather than never initializing it.
