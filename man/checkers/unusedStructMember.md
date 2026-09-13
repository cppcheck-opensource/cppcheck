# unusedStructMember

**Message**: struct member 'Point::unusedField' is never used.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

A struct/class/union member is never referenced anywhere in the file.

## Motivation

An unused struct member usually means the field can be removed, or - just as often - that it *should*
be used somewhere and isn't, which is worth a second look.

## How to fix

Remove the field, or actually use it.

Before:
```cpp
struct Point {
    int x;
    int y;
    int unusedField; // <- never referenced anywhere
};
```

After:
```cpp
struct Point {
    int x;
    int y;
};
```

## Related checkers

- [unusedVariable.md](unusedVariable.md) - the same idea, for a local variable instead of a struct
  member.
