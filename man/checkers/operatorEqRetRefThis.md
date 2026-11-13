# operatorEqRetRefThis

**Message**: 'operator=' should return reference to 'this' instance.<br/>
**Category**: Correctness<br/>
**Severity**: Style<br/>
**Language**: C++

## Description

A hand-written `operator=` doesn't `return *this;` - code like `a = b = c;` or chaining `.` after an
assignment then breaks, since the return value isn't what everyone expects.

## Motivation

Idiomatic C++ assignment operators return a reference to the assigned-to object so that assignments can
be chained (`a = b = c;`) and the result can be used directly. An `operator=` that returns something
else (or nothing meaningful) silently breaks that convention for any caller who relies on it, which is
easy to miss since a single, non-chained `a = b;` still compiles and works either way.

## How to fix

Return `*this` from `operator=`.

Before:
```cpp
class A {
public:
    A & operator=(const A &a) { return a; } // <- returns 'a', not '*this'
};
```

After:
```cpp
class A {
public:
    A & operator=(const A &a) { return *this; }
};
```

## Related checkers

- [operatorEqMissingReturnStatement.md](operatorEqMissingReturnStatement.md) - the more severe sibling
  case where `operator=` has no `return` statement at all.
- [operatorEqShouldBeLeftUnimplemented.md](operatorEqShouldBeLeftUnimplemented.md) - the case where
  `operator=` deliberately never returns, and should instead be declared unimplemented/deleted.
