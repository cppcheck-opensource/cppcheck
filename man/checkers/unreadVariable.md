# unreadVariable

**Message**: Variable 'x' is assigned a value that is never used.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

A variable is assigned a value (or modified), but that value is never read afterwards - so the
assignment has no effect.

## Motivation

An assignment whose value is never read is pointless code that adds noise for readers, and is often a
sign of a bug: a forgotten follow-up use of the variable, a typo'd variable name on the next line, or
dead code left over from a refactor.

## How to fix

Remove the pointless assignment, or actually read the value afterwards.

Before:
```cpp
void f() {
    int x = 5; // <- never read afterwards
}
```

After:
```cpp
void f() {
    int x = 5;
    print(x);
}
```

## False positives to be aware of

- **An RAII guard object bound through `auto&&` can be wrongly reported as an unused assignment.** An
  object whose entire purpose is its constructor/destructor side effect (for example a
  `std::lock_guard`) has no "value" to read back, but is not just an unused assignment either:
  ```cpp
  #include <mutex>
  void f(std::mutex& mutex) {
      auto&& g = std::lock_guard<std::mutex>{ mutex }; // reported as unread, but 'g' locks/unlocks the mutex
  }
  ```

## Related checkers

- [unusedVariable.md](unusedVariable.md) - the sibling finding for a variable that's never read *or*
  written at all.
- [unassignedVariable.md](unassignedVariable.md) - the opposite situation: a variable read before it's
  ever assigned.
