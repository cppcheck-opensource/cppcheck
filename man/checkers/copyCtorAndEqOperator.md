# copyCtorAndEqOperator

**Message**: The class 'A' has 'copy constructor' but lack of 'operator='.<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C++

## Description

A class defines a copy constructor without also defining `operator=`, or vice versa - having a working
copy in one direction and the compiler-generated (possibly wrong) one in the other is an easy way to end
up with a broken half-copy.

**This check is currently switched off in cppcheck** (its message was found to need clarification), so
it will not appear in current output no matter what code you give it. It's documented here only because
it remains a real, registered ID.

## Motivation

If a class needs custom copying logic for its copy constructor, it almost always needs the same logic
for `operator=` (and vice versa) - the compiler-generated version of whichever one is missing just
copies each member's value, which usually doesn't match the custom behaviour the other one implements.

## Related checkers

- [noCopyConstructor.md](noCopyConstructor.md) and [noOperatorEq.md](noOperatorEq.md) - the more
  commonly-seen checks for a class that's missing one of these special member functions entirely,
  rather than defining one without the other.
