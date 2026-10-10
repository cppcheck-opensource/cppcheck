# knownConditionTrueFalse

**Message**: Condition 'x==5' is always true<br/>
**Category**: Code cleanup<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

A condition is always true or always false.

Note: a warning is not written for obvious cases like `if (false)`.

If a condition is always true then technically the condition is redundant. It
can be removed so that the conditional code will be unconditionally executed.
This reduces complexity.

If a condition is always false then the conditional code is unreachable and can
be removed.

It is however also possible that the intended check never actually happens and
a real bug (a wrong comparison, a typo'd variable, a value that was supposed to
vary but doesn't) slips through unnoticed.

## Motivation

The condition may be invariant (always true or always false) by mistake,
otherwise it is possible to cleanup redundant code to reduce complexity.

## How to fix

Before (condition is always true):
```cpp
void f() {
    int x = 5;
    if (x == 5) { // <- always true
      dostuff();
    }
}
```

After: The condition is technically redundant, this code is logically the same.
```cpp
void f() {
    dostuff();
}
```

Before (condition is always false):
```cpp
void f() {
    int x = 5;
    if (x < 3) {  // <- always false
      dostuff();
    }
}
```

After: The conditional code is unreachable, this code is logically the same.
```cpp
void f() {
}
```

## Related checkers

- [assignIfError.md](assignIfError.md) - the same idea, specifically for a value just narrowed down by
  a bitmask assignment.
- [moduloAlwaysTrueFalse.md](moduloAlwaysTrueFalse.md), [compareValueOutOfTypeRangeError.md](compareValueOutOfTypeRangeError.md) -
  the same idea, for a `%` result or a type's value range instead of a general known value.
- [duplicateConditionalAssign.md](duplicateConditionalAssign.md) - a related, narrower case: an
  assignment that repeats a value a condition already guarantees.
