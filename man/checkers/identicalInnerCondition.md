# identicalInnerCondition

**Message**: Identical inner 'if' condition is always true.<br/>
**Category**: Code Quality<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

An `if` nested directly inside another `if` repeats the exact same condition as the outer one, so the
inner condition is always true.

## Motivation

If the outer `if` already tested the condition, testing it again immediately inside is redundant - the
inner `if` can never be false, so it adds nothing but confusion (and cost of a reader wondering whether
something subtle was intended).

This check may need `--check-level=exhaustive` to see every case.

## How to fix

Before:
```cpp
void f(int x) {
    if (x == 1) {
        if (x == 1) {} // <- always true
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

- [oppositeInnerCondition.md](oppositeInnerCondition.md) - the same idea, but the inner condition
  contradicts the outer one instead of repeating it.
- [overlappingInnerCondition.md](overlappingInnerCondition.md) - the same idea, but the inner condition
  is merely implied by (not identical to) the outer one.
- [multiCondition.md](multiCondition.md) - the same idea, but for an `if`/`else if` chain instead of
  nested `if`s.
