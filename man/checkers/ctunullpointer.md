# ctunullpointer

**Message**: Null pointer dereference: p<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error/Warning<br/>
**Language**: C/C++

## Description

The same idea as [nullPointer](nullPointer.md), but found by cppcheck's whole-program ("cross
translation unit") analysis, which follows a null value across several function calls that the normal,
per-function analysis doesn't always chain together on its own.

## Motivation

Some null-pointer bugs only become visible when tracing a value across function boundaries - for
example, a function that's given a null pointer by one of its callers, several call levels away. This
whole-program analysis catches those cases at the cost of needing every relevant source file analyzed
together. Because of this, the same bug can sometimes be reported twice - once as a plain `nullPointer`
and once as `ctunullpointer` - for the same line.

## Related checkers

- [nullPointer.md](nullPointer.md) - the per-function analysis this whole-program analysis complements.
- [ctunullpointerOutOfMemory.md](ctunullpointerOutOfMemory.md) - the same idea for a pointer from a failable allocation function.
