# identicalConditionAfterEarlyExit

**Message**: Identical condition 'x==1', second condition is always false<br/>
**Category**: Code Quality<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

After `if (cond) return;` (or `throw`/`continue`/`break`), the same condition is tested again later in
the function - by that point it can only be false.

## Motivation

Once execution passes the early exit, the condition that would have triggered it is already known to be
false for the rest of the function - re-testing it is always false, so it's either dead code or a sign
the two conditions were meant to check different things.

This check may need `--check-level=exhaustive` to see every case.

## How to fix

Before:
```cpp
void f(int x) {
    if (x == 1)
        return;
    if (x == 1) {} // <- always false here
}
```

After:
```cpp
void f(int x) {
    if (x == 1)
        return;
}
```

## Related checkers

- [duplicateCondition.md](duplicateCondition.md) - the same idea, but for two consecutive plain `if`
  statements with no early exit in between.
