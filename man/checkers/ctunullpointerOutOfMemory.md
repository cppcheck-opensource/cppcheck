# ctunullpointerOutOfMemory and ctunullpointerOutOfResources

**Message**: If memory allocation fails, then there is a possible null pointer dereference: p<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error/Warning<br/>
**Language**: C/C++

## Description

- `ctunullpointerOutOfMemory`: the same idea as [nullPointerOutOfMemory](nullPointerOutOfMemory.md),
  but found by cppcheck's whole-program ("cross translation unit") analysis, which follows the result
  of a failable memory allocation across function calls that the normal, per-function analysis doesn't
  always chain together on its own.
- `ctunullpointerOutOfResources`: the same idea for a failable resource-allocating function (`fopen`,
  ...) rather than a memory allocator.

## Motivation

Some "forgot to check the allocation" bugs only become visible when tracing the allocated pointer/handle
across function boundaries. This whole-program analysis catches those cases at the cost of needing every
relevant source file analyzed together. Because of this, the same bug can sometimes be reported twice -
once as a plain `nullPointerOutOfMemory`/`nullPointerOutOfResources` and once with the `ctu` prefix - for
the same line.

## Related checkers

- [nullPointerOutOfMemory.md](nullPointerOutOfMemory.md) - the per-function analysis this whole-program analysis complements.
- [ctunullpointer.md](ctunullpointer.md) - the same whole-program analysis without a specific failable allocation involved.
