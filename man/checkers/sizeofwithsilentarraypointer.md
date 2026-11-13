# sizeofwithsilentarraypointer

**Message**: Using 'sizeof' on array given as function argument returns size of a pointer.<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

`sizeof` is applied to an array that was received as a function parameter - which has already decayed
to a plain pointer, so the result is the pointer's size, not the array's.

## Motivation

An array parameter (`int a[10]`) is really just a pointer as far as the function body is concerned - the
size information from the declaration is not available at runtime. `sizeof(a)` inside the function
silently returns the size of the pointer, not the number of bytes the caller's array actually occupies,
which is easy to miss since the declaration still visually looks like an array.

## How to fix

Before:
```cpp
void f(int a[10]) {
    int n = sizeof(a) / sizeof(int); // <- 'a' decayed to int*, this is not 10
}
```

After:
```cpp
void f(int a[10], int n) {
    int m = n / sizeof(int); // pass the real element count instead
}
```
