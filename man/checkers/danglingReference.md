# danglingReference

**Message**: Using reference to dangling temporary.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C++

## Description

A reference variable that itself outlives the function (`static`, or a non-local reference parameter)
is bound to a local variable.

## Motivation

A reference is just another name for the object it's bound to - it doesn't keep that object alive. If
the reference itself outlives the local variable it was bound to, using it afterwards is undefined
behaviour, in the same way a dangling pointer would be.

## How to fix

Bind the long-lived reference to something that actually outlives the function (a `static` variable, a
global, or something the caller owns), not a plain local variable.

## Related checkers

- [danglingLifetime.md](danglingLifetime.md) - the same idea, for a pointer instead of a reference.
- [danglingTempReference.md](danglingTempReference.md) - a related mistake where a reference is bound
  to a temporary and used after that temporary has been destroyed, within the same function.
