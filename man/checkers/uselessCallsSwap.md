# uselessCallsSwap

**Message**: It is inefficient to swap a object with itself by calling 'x.swap(x)'<br/>
**Category**: Performance<br/>
**Severity**: Performance<br/>
**Language**: C++

## Description

`x.swap(x)` - calling `.swap()` with the object itself as the argument is an inefficient no-op -
usually a copy-paste mistake where the wrong variable was used for one side.

## Motivation

Swapping a value with itself has no effect: the call still does the work of a swap (temporaries,
moves) for a result that's guaranteed to be identical to not calling it at all. The fact that it was
written at all suggests the intent was to swap with a different variable, one of which was mistyped.

## How to fix

Before:
```cpp
#include <string>
void f() {
    std::string s1, s2;
    s2.swap(s2); // <- no effect, just wasted work
}
```

After:
```cpp
#include <string>
void f() {
    std::string s1, s2;
    s1.swap(s2);
}
```

## Related checkers

- [uselessCallsCompare.md](uselessCallsCompare.md) - the same self-argument mistake, for `.compare()`
  instead of `.swap()`.
