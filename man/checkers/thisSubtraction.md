# thisSubtraction

**Message**: Suspicious pointer subtraction. Did you forget to dereference it?<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C++

## Description

The code contains `this-x` - almost always a typo for `this->x`, since subtracting an arbitrary value
from `this` as a pointer is rarely meaningful.

## Motivation

`this-x` and `this->x` look almost identical but mean completely different things: one does pointer
arithmetic on `this` itself (rarely intended), the other accesses member `x`. This is an easy typo to
make and an easy one to miss when reading over code quickly.

It's also not merely a readability problem: `this` points to a single object, not an array, so subtracting
any nonzero value from it produces a pointer outside that object's bounds, which the standard's pointer
arithmetic rules make undefined behaviour to even form, before it's ever dereferenced or compared. cppcheck
doesn't evaluate `x` here, though - it flags the `this - x` syntax on sight, regardless of what `x` actually
is, so this is reported purely as a likely typo for `this->x`, not because cppcheck has confirmed the
subtraction itself is unsafe.

## How to fix

Before:
```cpp
class C {
public:
    int x;
    void f() {
        this-x; // <- thisSubtraction: likely meant 'this->x'
    }
};
```

After:
```cpp
class C {
public:
    int x;
    void f() {
        this->x;
    }
};
```
