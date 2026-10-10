# sizeofFunctionCall

**Message**: Found function call inside sizeof().<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

A function call appears inside `sizeof`, e.g. `sizeof(foo())` - `foo()` is never actually called.

## Motivation

The operand of `sizeof` is normally not evaluated at all - only its type is used to compute the result.
A function call written inside `sizeof` reads as if it runs (and, worse, hides a call whose return
value or side effect might have been genuinely needed), but it silently never executes.

## How to fix

Before:
```cpp
int compute();
void f() {
    int s = sizeof(compute()); // <- compute() is never called
}
```

After:
```cpp
int compute();
void f() {
    int s = sizeof(decltype(compute()));
}
```
