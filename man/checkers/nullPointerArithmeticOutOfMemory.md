# nullPointerArithmeticOutOfMemory

**Message**: If memory allocation fails: pointer addition with NULL pointer.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C/C++

## Description

Pointer arithmetic is performed on a pointer that came from a memory-allocation function that can fail
and return null (`malloc`, `new` in some configurations, ...), without checking for that failure first.

## Motivation

Just like [nullPointerOutOfMemory](nullPointerOutOfMemory.md), assuming an allocation always succeeds
means that under memory pressure, the very next operation on the result - even pointer arithmetic that
doesn't dereference anything - is undefined behaviour.

## Related checkers

- [nullPointerArithmetic.md](nullPointerArithmetic.md) - the same idea without a specific failable allocation involved.
- [nullPointerOutOfMemory.md](nullPointerOutOfMemory.md) - the direct-dereference equivalent of this check.
