# redundantPointerOp

**Message**: Redundant pointer operation on 'p' - it's already a pointer.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

`&*p` or `*&p` is used where `p` is already exactly what's needed - taking the address of a dereference
(or vice versa) just to undo it.

## Motivation

This pattern has no effect beyond what writing `p` directly would achieve, and only adds noise that
makes a reader wonder if there's a subtlety being expressed that isn't actually there.

## How to fix

Before:
```cpp
int *f(int *x) {
    return &*x; // <- 'x' is already a pointer
}
```

After:
```cpp
int *f(int *x) {
    return x;
}
```
