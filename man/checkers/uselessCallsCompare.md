# uselessCallsCompare

**Message**: It is inefficient to call 'x.compare(x)' as it always returns 0.<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C++

## Description

`x.compare(x)` - calling `.compare()` with the object itself as the argument always returns `0`, which
looks like a meaningful check but never is - usually a copy-paste mistake where the wrong variable was
used for one side.

## Motivation

Comparing a value against itself is always true (or, for `.compare()`, always `0`) - the call is dead
code that adds nothing, and the fact that it was written at all suggests the intent was to compare two
different values, one of which was mistyped.

## How to fix

Before:
```cpp
#include <string>
void f() {
    std::string s1, s2;
    s2.compare(s2); // <- always returns 0
}
```

After:
```cpp
#include <string>
void f() {
    std::string s1, s2;
    s1.compare(s2);
}
```

## Related checkers

- [uselessCallsSwap.md](uselessCallsSwap.md) - the same self-argument mistake, for `.swap()` instead of
  `.compare()`.
