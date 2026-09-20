# assignBoolToPointer

**Message**: Boolean value assigned to pointer.<br/>
**Category**: Correctness/Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C++

## Description

A `bool` value is assigned directly to a pointer variable.

## Motivation

Assigning a `bool` to a pointer almost never expresses what a reader would expect a pointer assignment
to mean, and is usually a typo for something like assigning through the pointer (`*p = flag;`) instead
of to it.

The two possible values are not equally risky. Assigning `false` is harmless: it converts to `0`, which
is always a well-defined way to give the pointer a null value - purely a readability/intent problem, not
a safety one. Assigning `true` is different: in C++ this pattern is normally rejected outright by the
compiler; in C it is typically accepted (at most with a warning) as an implicit conversion of the value
`1` to a pointer, which the C standard leaves implementation-defined - the resulting pointer is not
guaranteed to be aligned or to point to anything real. If that pointer is later dereferenced, the
dereference itself is undefined behaviour, because the pointer doesn't actually point to a valid object
of its type. Since the assigned value is often a variable rather than a literal, either outcome is
possible depending on what the variable holds at runtime.

## How to fix

Before:
```cpp
bool flag;
bool *p;
void f() {
    p = flag; // <- likely meant '*p = flag;'
}
```

After:
```cpp
bool flag;
bool *p;
void f() {
    *p = flag;
}
```
