# oppositeInnerCondition

**Message**: Opposite inner 'if' condition leads to a dead code block.<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

An `if` nested directly inside another `if` has a condition that contradicts the outer one, so its body
is dead code.

## Motivation

Once the outer condition is true, the value(s) it depends on are already narrowed down; an inner
condition that could only be true when the outer one is false can never actually run. This usually means
either dead code left over from an edit, or a comparison that doesn't say what the author meant.

This check may need `--check-level=exhaustive` to see every case.

## How to fix

Before:
```cpp
void f(int x) {
    if (x == 1) {
        if (x == 2) {} // <- dead code, x is already 1 here
    }
}
```

After: remove the dead inner check.
```cpp
void f(int x) {
    if (x == 1) {
    }
}
```

## Related checkers

- [identicalInnerCondition.md](identicalInnerCondition.md) - the same idea, but the inner condition
  repeats the outer one exactly instead of contradicting it.
- [overlappingInnerCondition.md](overlappingInnerCondition.md) - the same idea, but the inner condition
  is already implied by (not identical to) the outer one.
- [multiCondition.md](multiCondition.md) - the same idea, but for an `if`/`else if` chain instead of
  nested `if`s.
