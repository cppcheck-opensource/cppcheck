# operatorEqVarError

**Message**: Member variable 'classname::varname' is not assigned a value in 'classname::operator='.<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C++

## Description

Same idea as [missingMemberCopy.md](missingMemberCopy.md), but for a hand-written `operator=` instead
of a copy constructor: one particular member is never assigned anywhere in it, even though everything
else about the class suggests it should have been.

## Motivation

A hand-written `operator=` that forgets one member leaves that member holding its old value after an
assignment that's supposed to make the object equal to another - a subtle bug that's easy to miss since
the operator still compiles and mostly "looks right."

## How to fix

Assign the missing member too.

Before:
```cpp
struct S {
    int i{};
    S() = default;
    S& operator=(const S& s) { return *this; } // <- 'i' isn't assigned
};
```

After:
```cpp
struct S {
    int i{};
    S() = default;
    S& operator=(const S& s) { i = s.i; return *this; }
};
```

## Related checkers

- [missingMemberCopy.md](missingMemberCopy.md) - the same idea, for a hand-written copy/move constructor
  instead of `operator=`.
