# constStatement

**Message**: Redundant code: Found a statement that has no effect.<br/>
**Category**: Code Quality<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

A statement is just a constant, a variable name, or another expression with no side effect, and its
result is discarded - the statement does nothing.

## Motivation

A statement that's evaluated purely for its result, with that result then thrown away, is either dead
code left over from editing (for example a comparison that was meant to be an `if`) or a missing
function call (a `;` typed where `foo();` was meant).

## How to fix

Before:
```cpp
void f(int x) {
    x; // <- result discarded, has no effect
}
```

After:
```cpp
#include <cstdio>
void f(int x) {
    printf("%d", x);
}
```
