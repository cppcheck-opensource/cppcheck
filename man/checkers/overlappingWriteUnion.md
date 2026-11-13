# overlappingWriteUnion

**Message**: Overlapping read/write of union is undefined behavior<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C/C++

## Description

A union member is written using the value of a different, overlapping member of the same union in the
same expression (for example `u.i = u.f;`) - the read and write overlap in memory in a way the compiler
isn't required to sequence safely.

## Motivation

All members of a union share the same storage. Reading one member while writing a different, overlapping
one in the same expression is undefined behaviour: the compiler is free to assume the read and write
don't alias, and can reorder or optimize the expression in ways that don't match what the code visually
appears to do.

## How to fix

Read the source member into a separate variable first, then write the destination member from that
variable.

Before:
```cpp
void foo() {
    union { int i; float f; } u;
    u.i = 0;
    u.i = u.f; // <- reads and writes the same union storage in one expression
}
```

After:
```cpp
void foo() {
    union { int i; float f; } u;
    u.i = 0;
    float g = u.f;
    u.i = (int)g;
}
```

## Related checkers

- [overlappingWriteFunction.md](overlappingWriteFunction.md) - the same underlying overlapping-read/write
  hazard, but for a function call (like `memcpy()`) given overlapping source/destination ranges instead
  of a union member access.
- [UnionZeroInit.md](UnionZeroInit.md) - a different union-related pitfall, about a union not being
  fully zeroed by its initializer.
