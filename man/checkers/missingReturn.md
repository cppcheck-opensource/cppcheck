# missingReturn

**Message**: Found an exit path from function with non-void return type that has missing return statement<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C/C++

## Description

A function with a non-`void` return type has a path that reaches the end of the function (or a
`case`/`if` branch) without returning a value.

## Motivation

Falling off the end of a value-returning function without a `return` is undefined behaviour the moment
that code path is actually taken - the standard says so unconditionally, regardless of whether the
caller ever reads the returned value. Since the checker flags the mere existence of such a path, whether
this is reached in practice depends on the arguments/state a real call ends up using; when it is
reached, the caller receives whatever happened to be in the return-value register/location, which is
unpredictable and can differ between builds, platforms, or optimization levels.

## How to fix

Before:
```cpp
int f(int x) {
    if (x) {
        return 1;
    }
} // <- no return if 'x' is 0
```

After:
```cpp
int f(int x) {
    if (x) {
        return 1;
    }
    return 0;
}
```

