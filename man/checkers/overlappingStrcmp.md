# overlappingStrcmp

**Message**: The expression 'strcmp(x, "b") != 0' is suspicious. It overlaps 'strcmp(x, "a") == 0'.<br/>
**Category**: Code Quality<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

`strcmp(x, "a") == 0 || strcmp(x, "b") != 0` - the `!= 0` half is always true whenever the `== 0` half
is false, since `x` can't equal `"a"` and simultaneously differ from `"a"` in the case that makes the
first half false. In other words, the second condition doesn't add anything meaningful and is likely a
copy-paste mistake.

## Motivation

An `||` of two `strcmp()` checks like this always evaluates to true regardless of what `x` actually is,
which is easy to miss since each half looks like a reasonable, independent check on its own.

## How to fix

Before:
```cpp
void f(const char *str) {
    if (strcmp(str, "a") == 0 || strcmp(str, "b") != 0) {} // <- always true
}
```

After:
```cpp
void f(const char *str) {
    if (strcmp(str, "a") == 0 || strcmp(str, "c") == 0) {}
}
```

## Related checkers

- [sprintfOverlappingData.md](sprintfOverlappingData.md) - an unrelated string-function misuse in the
  same checker, about `sprintf()`'s destination and source arguments overlapping.
