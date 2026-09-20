# unusedVariable

**Message**: Unused variable: x<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

A variable is declared but never read or written at all.

## Motivation

A variable that's never touched after being declared is dead code: it adds to what has to be read and
maintained without doing anything, and is often a sign of a bug - a typo'd variable name, or code left
over from a refactor that no longer does what it used to.

## How to fix

Remove the variable, or actually use it.

Before:
```cpp
void f() {
    int x = 5; // <- declared and never read or written again
}
```

After:
```cpp
void f() {
    int x = 5;
    print(x);
}
```

## Related checkers

- [unreadVariable.md](unreadVariable.md) - the sibling finding for a variable that *is* assigned a
  value, but that value is never read afterwards.
- [unusedAllocatedMemory.md](unusedAllocatedMemory.md) - the same idea, specifically for memory obtained
  from an allocation function.
- [unassignedVariable.md](unassignedVariable.md) - the opposite situation: a variable read before it's
  ever assigned.
- [unusedStructMember.md](unusedStructMember.md) - the same idea, for a struct/class/union member
  instead of a local variable.
