# unusedPrivateFunction

**Message**: Unused private function: 'Fred::f'<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C++

## Description

A private member function is never called from anywhere in the class (or a friend of it).

## Motivation

Since a private function can only ever be called from inside its own class (or a friend), one that's
never called anywhere in the visible code is dead code - it can be removed without affecting any
caller outside the class.

## How to fix

Before:
```cpp
class Fred {
private:
    unsigned int f(); // <- unusedPrivateFunction: never called
public:
    Fred();
};
Fred::Fred() { }
unsigned int Fred::f() { return 1; }
```

After:
```cpp
class Fred {
private:
    unsigned int f();
public:
    Fred();
};
Fred::Fred() { f(); }
unsigned int Fred::f() { return 1; }
```
