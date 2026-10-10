# assignmentInAssert

**Message**: Assert statement modifies 'x'.<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

The assert expression assigns to, or increments/decrements, a variable that is also used outside the
assert - for example `assert(x = compute());`.

This checker only runs when the `warning` severity is enabled.

## Motivation

`assert()` is compiled out entirely in release builds (when `NDEBUG` is defined). Any effect placed
inside it - such as assigning to a variable that's relied on afterwards - only happens in debug builds
and silently vanishes in release builds. This is a classic source of code that "works when debugging"
and breaks (or does nothing) in the shipped build.

## How to fix

Perform the assignment outside the `assert()`, and assert on the already-computed result.

Before:
```cpp
void f(int y) {
    int x;
    assert(x = y + 1); // <- 'x' only gets its value in debug builds
}
```

After:
```cpp
void f(int y) {
    int x;
    x = y + 1;
    assert(x == 1);
}
```

## Related checkers

- [assertWithSideEffect.md](assertWithSideEffect.md) - the same idea, but for a function call inside
  `assert()` that may have a side effect, rather than a direct assignment.
