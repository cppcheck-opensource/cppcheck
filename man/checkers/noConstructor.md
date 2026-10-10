# noConstructor

**Message**: The struct 'x' does not have a constructor although it has private member variables.<br/>
**Category**: Correctness<br/>
**Severity**: Style<br/>
**Language**: C++

## Description

A class/struct has private member variables (of a native type, pointer, or a type that itself needs
initialization) but declares no constructor at all - those members are left with whatever bytes happened
to already be there when the object is created.

## Motivation

Without a constructor, a class's native-type, pointer, or reference members are left uninitialized when
an object is created - reading one of them before it's explicitly assigned would be undefined behaviour.
This checker only looks at the class's shape (private members, no constructor, no default member
initializer); it doesn't verify that any member is actually read before being set anywhere in the
program, so it flags classes that are technically at risk even if none of them are ever used that way in
practice. This check is also deliberately reported at `style` severity rather than `warning`: for
performance reasons a constructor might be intentionally left out in some cases, so it's presented as a
suggestion rather than a certain bug.

## How to fix

Add a constructor that initializes every member.

Before:
```cpp
class Fred {
    int i;
public:
    void setValue(int i_) { i = i_; } // <- no constructor initializes 'i'
};
```

After:
```cpp
class Fred {
    int i;
public:
    Fred() : i(0) {}
    void setValue(int i_) { i = i_; }
};
```

## Related checkers

- [uninitMemberVar.md](uninitMemberVar.md) - the related family of checks for when a constructor exists
  but still misses initializing one or more specific members.
