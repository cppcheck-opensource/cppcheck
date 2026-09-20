# nullPointerRedundantCheck

**Message**: Either the condition 'p' is redundant or there is possible null pointer dereference.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

A pointer is dereferenced, and elsewhere in the code there is a `NULL`/`nullptr` check on the very same
pointer - so either that check is redundant, or the dereference is a bug. This is often more useful
than a plain null-dereference warning, since it also catches "check after use" and "check the wrong
branch" mistakes, not just a directly-assigned null value.

## Motivation

A null check that exists somewhere in the code, but doesn't actually guard the dereference, is a strong
sign of a logic error: either the check was misplaced relative to the code it was meant to protect, or
the check itself is unnecessary and hides the fact that the pointer can never legitimately be null.
cppcheck only connects a check to a dereference when it can follow that the two really refer to the
same pointer value; once the pointer is reassigned, cached in a `bool`, or handed to another function
in between, that connection can be lost, so this check finding nothing is not proof the pointer is
always guarded correctly.

## How to fix

Before:
```cpp
void f(int *p) {
    *p = 1;    // <- dereferenced here...
    if (p) {}  // ...but only checked for null here
}
```

After:
```cpp
void f(int *p) {
    if (!p)
        return;
    *p = 1;
}
```

## Related checkers

- [nullPointer.md](nullPointer.md) - the general null-dereference check this one refines with a nearby-check cross-reference.
- [nullPointerArithmeticRedundantCheck.md](nullPointerArithmeticRedundantCheck.md) - the same idea for pointer arithmetic instead of a direct dereference.
