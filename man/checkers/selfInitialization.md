# selfInitialization

**Message**: Member variable 'i' is initialized by itself.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C++

## Description

A member is initialized with itself in the initializer list (`Fred() : i(i) {}` with no same-named
constructor parameter) - it reads its own not-yet-initialized value.

## Motivation

`i(i)` in a member-initializer list refers to the member `i` on both sides (there's no constructor
parameter with that name to shadow it) - the member ends up "initialized" by reading its own
uninitialized value, which is undefined behaviour and almost certainly a typo for a differently-named
constructor parameter.

## How to fix

Give the constructor parameter a distinct name, or otherwise supply a real initial value.

Before:
```cpp
class Fred {
    int i;
public:
    Fred() : i(i) { // <- reads its own uninitialized value
    }
};
```

After:
```cpp
class Fred {
    int i;
public:
    Fred() : i(0) {
    }
};
```

## Related checkers

- [initializerList.md](initializerList.md) - the related, broader family of member-initializer-list
  ordering pitfalls.
