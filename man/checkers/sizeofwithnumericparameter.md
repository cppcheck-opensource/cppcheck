# sizeofwithnumericparameter

**Message**: Suspicious usage of 'sizeof' with a numeric constant as parameter.<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

`sizeof` is applied to a plain numeric constant, e.g. `sizeof(10)`.

## Motivation

`sizeof` is nearly always resolved at compile time from the *type* of its operand, not its value - a
numeric literal's type is whatever the compiler infers for it (usually `int`), not something meaningful
to size a buffer or count elements by. This is almost always a mistake for a type name, or a variable
whose size was actually intended.

## How to fix

Before:
```cpp
void f() {
    int size = sizeof(10); // <- probably meant a type or a variable, not the literal 10
}
```

After:
```cpp
void f() {
    int size = sizeof(int);
}
```
