# multiCondition

**Message**: Expression is always false because 'else if' condition matches previous condition at line 2.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

An `else if` condition is the same as (dead code), or the exact opposite of (always true), the
condition already tested by the `if` before it.

## Motivation

By the time an `else if` is reached, the `if` before it is already known to be false. If the `else if`
condition is identical to that `if`, it can never be true either (dead code); if it's the exact
opposite, it's guaranteed to be true (redundant, since `else` alone would do the same thing).

## How to fix

Before:
```cpp
void f(int x) {
    if (x == 1) {}
    else if (x == 1) {} // <- can never be reached
}
```

After:
```cpp
void f(int x) {
    if (x == 1) {}
    else if (x == 2) {}
}
```

Before:
```cpp
void f(int x) {
    if (x == 1) {}
    else if (x != 1) {} // <- always true, we're already past the 'x == 1' branch
}
```

After:
```cpp
void f(int x) {
    if (x == 1) {}
    else {}
}
```

## Related checkers

- [duplicateCondition.md](duplicateCondition.md) - the same idea, but for two consecutive plain `if`
  statements rather than an `if`/`else if` chain.
- [oppositeInnerCondition.md](oppositeInnerCondition.md), [identicalInnerCondition.md](identicalInnerCondition.md) -
  the same idea again, but for an `if` nested directly inside another `if`.
