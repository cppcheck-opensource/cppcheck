# unhandledExceptionSpecification

**Message**: Unhandled exception specification when calling function thrower().<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C++ only

## Description

A function with no exception specification of its own calls another function that has an (old-style,
C++17-removed) `throw(SomeType)` dynamic exception specification, without any `try`/`catch` in between.

## Motivation

A dynamic exception specification like `throw(int)` documents that a function can only throw that
particular type - calling it without handling that possibility is worth a second look, especially since
this old-style specification was removed from the language in C++17 and callers may not realize it's
still meaningful in the code they're reading.

## How to fix

Before:
```cpp
void thrower() throw(int);
void f() {
    thrower(); // <- unhandled 'throw(int)' specification
}
```

After:
```cpp
void thrower() throw(int);
void f() {
    try {
        thrower();
    } catch (int) {
    }
}
```
