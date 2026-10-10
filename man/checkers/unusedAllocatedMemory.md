# unusedAllocatedMemory

**Message**: Variable 'p' is allocated memory that is never used.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

A variable is assigned the result of an allocation (`malloc`, `new`, ...), but the allocated memory
itself is never read or written - only, at most, freed again.

## Motivation

Allocating memory that's never actually used is pointless, and usually a sign that some code that was
meant to fill or use the buffer was never written, or was removed during a refactor without noticing the
allocation.

## How to fix

Actually use the allocated memory, or remove the allocation.

Before:
```cpp
void f() {
    char* p = (char*)malloc(10); // <- allocated but never touched
}
```

After:
```cpp
void f() {
    char* p = (char*)malloc(10);
    p[0] = 'x';
    free(p);
}
```

## Related checkers

- [unusedVariable.md](unusedVariable.md) - the general form of this finding, for any variable that's
  never read or written.
