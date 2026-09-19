# missingMemberCopy

**Message**: Member variable 'classname::varname' is not initialized in the copy constructor.<br/>
**Category**: Correctness<br/>
**Severity**: Warning (Inconclusive)<br/>
**Language**: C++

## Description

A copy or move constructor is defined (with a body), but one particular member is never assigned
anywhere in it - everything else about the class suggests it should have been copied.

## Motivation

A hand-written copy/move constructor that forgets one member produces objects whose copies silently
diverge from the original in that one field - a subtle bug, especially in a class with many members,
since the constructor still compiles and mostly "looks right."

## How to fix

Copy (or move) the missing member too.

Before:
```cpp
struct S {
    int i{};
    S() = default;
    S(const S& s) {} // <- 'i' isn't copied
};
```

After:
```cpp
struct S {
    int i{};
    S() = default;
    S(const S& s) : i(s.i) {}
};
```

## Related checkers

- [operatorEqVarError.md](operatorEqVarError.md) - the same idea, for a hand-written `operator=` instead
  of a copy/move constructor.
- [uninitMemberVar.md](uninitMemberVar.md) - the related family of checks for a member that's never
  initialized by a regular constructor at all.
