# duplicateBreak

**Message**: Consecutive return, break, continue, goto or throw statements are unnecessary.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

Two `return`/`break`/`continue`/`goto`/`throw` statements appear back to back - the second one can
never be reached.

## Motivation

Once the first of these statements runs, control leaves the current block immediately, so nothing
written directly after it (in the same block) can ever execute. It's dead code that's safe to delete.

## How to fix

Before:
```cpp
void foo(int a) {
    while (1) {
        if (a++ >= 100) {
            break;
            continue; // <- can never be reached
        }
    }
}
```

After:
```cpp
void foo(int a) {
    while (1) {
        if (a++ >= 100) {
            break;
        }
    }
}
```

## Related checkers

- [unreachableCode.md](unreachableCode.md) - the more general case of any code (not just another jump statement) placed after one of these statements.
