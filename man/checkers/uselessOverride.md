# uselessOverride

**Message**: The function 'f' is an unnecessary overload; it is identical to the parent function<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C++

## Description

An overriding function's body is either functionally identical to the base version, or just calls the
base version and returns its result - the override adds a function call for no behavioural difference.

## Motivation

An override that does nothing differently from the base class is dead weight: it adds an indirection
and a place for the two versions to accidentally drift apart later, without changing what the program
does today.

## How to fix

Before:
```cpp
struct B { virtual int f() { return 5; } };
struct D : B {
    int f() override { return B::f(); } // <- uselessOverride: identical to the base version
};
```

After: remove the pointless override entirely.
```cpp
struct B { virtual int f() { return 5; } };
struct D : B {
};
```

## Related checkers

- [missingOverride.md](missingOverride.md) - for a function that overrides a base one without being
  marked `override`, regardless of whether the override is useless.
