# unreachableCode

**Message**: Statements following return, break, continue, goto or throw will never be executed.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

Code appears right after a `return`/`break`/`continue`/`goto`/`throw`, or after a call to a function
that never returns - none of it can ever execute.

## Motivation

This is dead code by definition: control leaves the enclosing block before reaching it, so it can be
deleted with no change in behavior. It's also worth double-checking that the surrounding logic is
actually correct, since unreachable code sometimes reveals a misplaced statement.

## How to fix

Before:
```cpp
void bar();
void foo() {
    return;
    bar(); // <- can never run
}
```

After:
```cpp
void bar();
void foo() {
    bar();
    return;
}
```

## Related checkers

- [duplicateBreak.md](duplicateBreak.md) - the specific case where the unreachable statement is itself another `return`/`break`/`continue`/`goto`/`throw`.
- [unreachableSwitchCase.md](unreachableSwitchCase.md) - a `switch` `case` that can never be selected, rather than code placed after an exit statement.
