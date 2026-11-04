# copyCtorNoAllocation

**Message**: Copy constructor does not allocate memory for member 'p' although memory has been allocated in other constructors.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Warning<br/>
**Language**: C++

## Description

The intended sibling of [copyCtorPointerCopying.md](copyCtorPointerCopying.md): a copy constructor
copies a pointer without allocating new memory, checked from the allocating constructor's side instead
of the copy constructor's side.

**This check is dead code in current cppcheck.** Its implementation is commented out in the source
(citing an unresolved report about the message needing more work), so it can never actually fire, no
matter what code you give it. It's documented here only because it remains a real, registered ID.

## Motivation

If another constructor allocates memory for a pointer member, a copy constructor that doesn't do the
same for that member is expected to share the same shallow-copy/double-free risk that
[copyCtorPointerCopying.md](copyCtorPointerCopying.md) describes.

## Related checkers

- [copyCtorPointerCopying.md](copyCtorPointerCopying.md) - the actively-working check for the same
  underlying shallow-copy risk, checked from the copy constructor's side.
