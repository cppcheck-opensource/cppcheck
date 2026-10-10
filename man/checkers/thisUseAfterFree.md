# thisUseAfterFree

**Message**: Class member 'x' is accessed after deleting 'this' (or a smart pointer holding it).<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Warning<br/>
**Language**: C++

## Description

A member function `delete`s (or `.reset()`s) a pointer/smart-pointer that holds `this` itself, and then
goes on to call another method or use another member - by that point the object has already been
destroyed.

## Motivation

Once an object has deleted itself (typically via a `static` "instance" pointer, or a smart pointer that
owns it), continuing to use `this` - or anything reached through it, including other member functions -
is a use-after-free, even though the memory may still look intact for a little while afterwards.

## How to fix

Before:
```cpp
class C {
public:
  void dostuff() { delete mInstance; hello(); } // <- thisUseAfterFree: 'this' is gone after the delete
private:
  static C *mInstance;
  void hello() {}
};
```

After:
```cpp
class C {
public:
  void dostuff() { hello(); delete mInstance; }
  void hello() {}
private:
  static C *mInstance;
};
```
