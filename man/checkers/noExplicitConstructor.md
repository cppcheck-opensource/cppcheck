# noExplicitConstructor

**Message**: Class 'Class' has a constructor with 1 argument that is not explicit.<br/>
**Category**: Correctness<br/>
**Severity**: Style<br/>
**Language**: C++

## Description

A constructor that takes exactly one argument (and so can be used for an implicit conversion) isn't
marked `explicit` - so a value of that argument's type can silently convert into the class wherever the
class type is expected, which is rarely intended.

## Motivation

A single-argument constructor that isn't `explicit` doubles as an implicit conversion: anywhere the
class type is expected, a value of the argument's type is silently accepted and converted, without the
reader seeing any indication a conversion happened at all. This can produce confusing overload
resolution and surprising implicit conversions that the author never intended.

## How to fix

Mark the constructor `explicit`, unless the implicit conversion is genuinely intended.

Before:
```cpp
class Class {
public:
    Class(int i) {} // <- not explicit
};
```

After:
```cpp
class Class {
public:
    explicit Class(int i) {}
};
```
