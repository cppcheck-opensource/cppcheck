# danglingTemporaryLifetime

**Message**: Using pointer to dangling temporary.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C++

## Description

A pointer or iterator into a temporary object is used after that temporary has already been destroyed.

## Motivation

A temporary object is normally destroyed at the end of the full expression that created it. A pointer
or iterator into it doesn't extend its lifetime the way a reference binding sometimes can - so using
one afterwards is a use of an already-destroyed object.

## How to fix

Only use a pointer/iterator into a temporary within the same full expression that created it, or store
the value itself (by copy or move) instead if it needs to outlive that expression.

## Related checkers

- [danglingTempReference.md](danglingTempReference.md) - the same idea, for a reference instead of a
  pointer/iterator.
- [invalidLifetime.md](invalidLifetime.md) - the same underlying "used after its referent's scope
  ended" mistake, for a pointer into a named local variable rather than a temporary.
