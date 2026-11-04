# dangerousTypeCast

**Message**: Potentially invalid type conversion in old-style C cast, clarify/fix with C++ cast<br/>
**Category**: Type Safety<br/>
**Severity**: Warning<br/>
**Language**: C++, not applicable for C code

## Description

An old-style C cast (`(Type)expr`) is used to convert between two pointer or reference types in a way
that could be an invalid conversion - for example casting between two unrelated pointer types, or down
a class hierarchy without any guarantee the object is really of the target type. A C-style cast will
silently do whatever C++ cast is needed to make the code compile (including a `reinterpret_cast`), so it
gives no indication of, or protection against, this.

## Motivation

C style casts can be dangerous in many ways:
 * loss of precision
 * loss of sign
 * loss of constness
 * invalid type conversion

## Philosophy

This checker warns about old style C casts that perform type conversions that can be invalid.

This checker is not about readability. It is about safety.

## How to fix

You can use `dynamic_cast` or `static_cast` to fix these warnings.

Before:
```cpp
struct Base{};
struct Derived: public Base {};
void foo(Base* base) {
    Derived *p = (Derived*)base; // <- can be invalid
}
```

After:
```cpp
struct Base{};
struct Derived: public Base {};
void foo(Base* base) {
    Derived *p = dynamic_cast<Derived*>(base);
}
```
