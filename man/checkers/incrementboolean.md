# incrementboolean

**Message**: Incrementing a variable of type 'bool' with postfix operator++ is deprecated by the C++ Standard. You should assign it the value 'true' instead.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C++

## Description

A `bool` variable is incremented with `++`.

## Motivation

Incrementing a `bool` is deprecated by the C++ standard (Annex D) and always just sets it to `true` -
writing `= true` says the same thing without relying on deprecated, integer-flavored behaviour of a
type that isn't really a number.

## How to fix

Before:
```cpp
bool ready = true;
void f() {
    ready++; // <- deprecated
}
```

After:
```cpp
bool ready = true;
void f() {
    ready = true;
}
```
