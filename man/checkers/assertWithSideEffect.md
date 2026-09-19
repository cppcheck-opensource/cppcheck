# assertWithSideEffect

**Message**: Assert statement calls a function which may have desired side effects: 'foo'.<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

The expression inside an `assert(...)` call calls a function that may have a side effect - for example
a non-`const` member function, or a function whose body is seen to modify a reference/pointer argument
it was given. When the function's body isn't visible, cppcheck falls back to a narrower guess based on
whether it's declared `const`/`static`, rather than warning about every unresolvable call - so this
check only flags a call when it has at least some concrete reason to suspect a side effect.

This checker only runs when the `warning` severity is enabled.

## Motivation

`assert()` is compiled out entirely in release builds (when `NDEBUG` is defined). Relying on a function
call inside it to actually do something only happens in debug builds and silently vanishes in release
builds. This is a classic source of code that "works when debugging" and breaks (or does nothing) in
the shipped build.

## How to fix

Perform the call outside the `assert()`, and assert on the already-computed result.

Before:
```cpp
struct Stack {
    int top = 0;
    bool pop(int& out) {
        if (top == 0)
            return false;
        out = --top;
        return true;
    }
};

void foo(Stack& s) {
    int value;
    assert(s.pop(value)); // <- pop() only runs in debug builds
}
```

After:
```cpp
struct Stack {
    int top = 0;
    bool pop(int& out) {
        if (top == 0)
            return false;
        out = --top;
        return true;
    }
};

void foo(Stack& s) {
    int value;
    bool popped = s.pop(value);
    assert(popped);
}
```

## Related checkers

- [assignmentInAssert.md](assignmentInAssert.md) - the same idea, but for a direct assignment inside
  `assert()` rather than a function call.
