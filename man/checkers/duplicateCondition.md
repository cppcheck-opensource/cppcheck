# duplicateCondition

**Message**: The if condition is the same as the previous if condition<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

Two consecutive `if` statements test the exact same condition - the second one is dead code.

## Motivation

If the first `if` didn't change anything the condition depends on, the second, identical `if` can never
newly become true or false compared to the first - it's either always dead code, or a sign that an
assignment that was supposed to happen in between is missing.

## How to fix

Before:
```cpp
void f(int x) {
    if (x == 1) {}
    if (x == 1) {} // <- dead code
}
```

After:
```cpp
void f(int x) {
    if (x == 1) {}
}
```

## Related checkers

- [multiCondition.md](multiCondition.md) - the same idea, but for `if`/`else if` chains.
- [identicalConditionAfterEarlyExit.md](identicalConditionAfterEarlyExit.md) - the same idea, but the
  first check is an early `return`/`throw`/`break`/`continue` instead of a plain `if`.
