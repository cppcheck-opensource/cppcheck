# overlappingInnerCondition

**Message**: Overlapping inner 'if' condition is always true.<br/>
**Category**: Code Quality<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

An `if` nested directly inside another `if` has a condition that's already guaranteed by the outer one
(a bitwise overlap, for example `x == 1` outside and `x & 7` inside), so the inner condition is always
true.

## Motivation

If the outer condition already guarantees the inner one, the inner `if` can never be false - it adds
nothing but a false impression that there's a real extra check happening.

This check may need `--check-level=exhaustive` to see every case.

## How to fix

Before:
```cpp
void f(int x) {
    if (x == 1) {
        if (x & 7) {} // <- always true, implied by x == 1
    }
}
```

After: remove the redundant inner check.
```cpp
void f(int x) {
    if (x == 1) {
    }
}
```

## Related checkers

- [identicalInnerCondition.md](identicalInnerCondition.md) - the same idea, but the inner condition is
  identical to the outer one rather than merely implied by it.
- [oppositeInnerCondition.md](oppositeInnerCondition.md) - the same idea, but the inner condition
  contradicts the outer one instead.
- [multiCondition.md](multiCondition.md) - the same idea, but for an `if`/`else if` chain instead of
  nested `if`s.
