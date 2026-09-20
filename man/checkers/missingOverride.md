# missingOverride

**Message**: The function 'f' overrides a function in a base class but is not marked with a 'override' specifier.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C++

## Description

A derived class's function has the same signature as a virtual function in its base class, but isn't
marked `override`.

## Motivation

Without `override`, if a later edit to the base class's signature accidentally stops a derived function
from being an override (a parameter type change, a typo in the name, ...), the derived function quietly
becomes an unrelated new function instead - and nothing warns about it, since it's valid C++ either way.
Marking overrides explicitly turns that silent mismatch into a compile error.

## How to fix

Before:
```cpp
class Base { virtual void f(); };
class Derived : Base { virtual void f(); }; // <- missingOverride: no 'override'
```

After:
```cpp
class Base { virtual void f(); };
class Derived : Base { void f() override; };
```

## Related checkers

- [uselessOverride.md](uselessOverride.md) - for when a function *is* correctly overriding a base one,
  but the override doesn't actually change any behaviour.
