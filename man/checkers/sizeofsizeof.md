# sizeofsizeof

**Message**: Calling 'sizeof' on 'sizeof'.<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

`sizeof(sizeof(x))` - always the same as `sizeof(size_t)`, regardless of what `x` is.

## Motivation

`sizeof(...)` itself evaluates to a `size_t` value, so `sizeof` of that result is always just
`sizeof(size_t)` - a fixed, platform-defined constant that has nothing to do with `x`. Writing this
almost always means one `sizeof` too many was typed.

## How to fix

Before:
```cpp
void f() {
    int a;
    int s = sizeof(sizeof(a)); // <- always sizeof(size_t), regardless of 'a'
}
```

After:
```cpp
void f() {
    int a;
    int s = sizeof(a);
}
```
