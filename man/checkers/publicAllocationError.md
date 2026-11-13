# publicAllocationError

**Message**: Possible leak in public function. The pointer 'x' is not deallocated before it is allocated.<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C++

## Description

A public member function's first action is to allocate a new value directly into a member pointer,
without checking or freeing whatever it already pointed to - calling this function on an
already-initialized object leaks the old value.

## Motivation

A public function can be called at any time, including on an object that already holds a previous
allocation in that member. Overwriting the member with a fresh allocation, without freeing what it
pointed to first, leaks the old value every time the function is called on an object that already has
one - which working code calling this function more than once will do.

## How to fix

Before:
```cpp
class A {
    int *p;
public:
    A() : p(nullptr) {}
    void init() { p = new int; } // <- leaks the old 'p' if init() is called twice
};
```

After:
```cpp
class A {
    int *p;
public:
    A() : p(nullptr) {}
    void init() {
        delete p;
        p = new int;
    }
};
```

## Related checkers

- [unsafeClassCanLeak.md](unsafeClassCanLeak.md) - a related class-ownership leak, where nothing ever
  frees an allocated member at all.
