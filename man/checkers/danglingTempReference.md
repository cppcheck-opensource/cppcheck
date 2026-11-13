# danglingTempReference

**Message**: Using reference to dangling temporary.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C++

## Description

A reference bound to a temporary is used after that temporary has already been destroyed.

## Motivation

A temporary object is normally destroyed at the end of the full expression that created it. A small set
of lifetime-extension rules let a `const&`/`&&` binding stretch that lifetime in some cases, but not all
- using the reference outside the case where extension actually applies is a use of an already-destroyed
object.

## How to fix

Only rely on a reference to a temporary within the same full expression that created it, or store the
value itself (by copy or move) instead of a reference to it if it needs to outlive that expression.

## Related checkers

- [returnTempReference.md](returnTempReference.md) - the same underlying temporary-lifetime mistake,
  specifically when the reference is returned from the function.
- [danglingTemporaryLifetime.md](danglingTemporaryLifetime.md) - the same idea, for a pointer or
  iterator into a temporary instead of a reference.
- [danglingReference.md](danglingReference.md) - a related mistake where a long-lived reference is
  bound to a local variable (rather than a temporary) that doesn't outlive it.
