# UnionZeroInit

**Message**: You are using memset() to initialize a union that also contains a member with a bigger size.<br/>
**Category**: Portability<br/>
**Severity**: Portability<br/>
**Language**: C/C++

## Description

A union is zero-initialized (`= {0}` or `= {}`), but its largest member isn't declared first - only the
first member is guaranteed to be fully written by that initializer, so the rest of the union's storage
(beyond the first member's size) may not actually end up zeroed.

## Motivation

An aggregate initializer for a union only initializes the first named member. If a smaller member is
listed first, the initializer only guarantees that member's bytes are zeroed - the remaining bytes,
which are only reachable through a later, larger member, are left with whatever was already in memory.
Code that expects the whole union to be zero can then read uninitialized bytes through the larger
member.

## How to fix

Declare the union's largest member first, so a `{0}`/`{}` initializer zeroes its entire storage.

Before:
```cpp
void foo() {
    union { char c; int i; } bad0 = {0}; // <- 'i' (the larger member) isn't first
}
```

After:
```cpp
void foo() {
    union { int i; char c; } good0 = {0};
}
```

## Related checkers

- [overlappingWriteUnion.md](overlappingWriteUnion.md) - a different union-related pitfall, about
  reading and writing two overlapping members in the same expression.
