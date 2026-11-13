# nullPointerArithmeticRedundantCheck

**Message**: Either the condition 'p' is redundant or there is overflow in pointer addition.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

Pointer arithmetic is performed on a pointer that could be null, and elsewhere in the code there is a
`NULL`/`nullptr` check on the very same pointer - so either that check is redundant, or the arithmetic
is a bug.

## Motivation

Just like [nullPointerRedundantCheck](nullPointerRedundantCheck.md), a null check that exists somewhere
in the code but doesn't actually guard the pointer arithmetic is a strong sign of a misplaced check or
an unnecessary one.

## Related checkers

- [nullPointerArithmetic.md](nullPointerArithmetic.md) - the same idea without a nearby check to cross-reference.
- [nullPointerRedundantCheck.md](nullPointerRedundantCheck.md) - the direct-dereference equivalent of this check.
