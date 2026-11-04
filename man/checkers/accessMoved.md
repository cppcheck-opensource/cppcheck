# accessMoved and accessForwarded

**Message**: Access of moved variable 'v'.<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C++ only

## Description

A variable is read after it has been passed to `std::move()` (`accessMoved`) or `std::forward()`
(`accessForwarded`) - by convention, once a value has been moved/forwarded from, its contents are
unspecified and shouldn't be relied on (except to reset or destroy it).

## Motivation

`std::move()`/`std::forward()` don't themselves do anything except change how the compiler treats the
expression - the actual "move" happens in whatever constructor or assignment operator the moved-from
value is subsequently passed into, and its effect on the source object is entirely up to that type. By
convention a moved-from object is left in a valid but unspecified state, so any code that reads its
value afterwards (rather than just reassigning or destroying it) is relying on something the language
doesn't guarantee.

## How to fix

Before:
```cpp
#include <utility>
struct A {};
void g(A a);
void f() {
    A a;
    g(std::move(a));
    g(std::move(a)); // <- accessMoved: 'a' was already moved from above
}
```

After:
```cpp
#include <utility>
struct A {};
void g(A a);
void f() {
    A a;
    g(std::move(a));
}
```

Before:
```cpp
#include <utility>
template<typename T>
void g(T&&);
template<typename T>
void f(T && t) {
    g(std::forward<T>(t));
    T s = t; // <- accessForwarded: 't' was already forwarded above
}
```

After:
```cpp
#include <utility>
template<typename T>
void g(T&&);
template<typename T>
void f(T && t) {
    T s = t;
    g(std::forward<T>(t));
}
```
