# cstyleCast

**Message**: C-style pointer casting<br/>
**Category**: Modernization<br/>
**Severity**: Style<br/>
**Language**: C++, not applicable for C code

## Description

Many developers feel that it's best to replace C casts with C++ casts.

There are several advantages with C++ casts:
 * they permit only certain conversions
 * they express the intent
 * they are easy to identify

This checker is about C casts that converts to/from a pointer or reference.

**Note:** More "noisy" warnings exists that warn about all C casts. For instance Clang has
`-Wold-style-cast` and there is also such checking in Misra C++.

Dangerous conversions are covered by other warnings so this ID `cstyleCast` is primarily about
writing warnings for casts that are currently safe.

## Motivation

The motivation of this checker is to modernize c++ code.

## How to fix

You can use C++ casts such as `static_cast` to fix these warnings.

The `dynamic_cast` should rarely be used to fix these warnings because dangerousTypeCast is
reported when that can be a good idea.

Before:
```cpp
struct Base{};
struct Derived: public Base {};
void foo(Derived* derived) {
    Base *p = (Base*)derived; // <- cstyleCast: casting up to a base class is always safe
}
```

After:
```cpp
struct Base{};
struct Derived: public Base {};
void foo(Derived* derived) {
    Base *p = static_cast<Base*>(derived);
}
```
The `static_cast` documents the intended direction of the cast and will fail to compile if the
class hierarchy ever changes in a way that makes it invalid.
