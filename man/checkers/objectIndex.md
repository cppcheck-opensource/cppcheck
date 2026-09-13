# objectIndex

**Message**: The address of variable 's.x' is accessed at non-zero index.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C/C++

## Description

The address of one specific member of a variable is taken and then indexed with a nonzero value,
reaching past that member into the rest of the object.

## Motivation

Indexing past the address of a single member relies on that member happening to be followed by other
data with a compatible layout - this isn't something the language guarantees (padding, member reordering
by the compiler in some cases, and unrelated following members all make it unreliable), so it's
undefined behaviour dressed up as if it worked.

## How to fix

Before:
```cpp
struct S { int x; int y; };
void f() {
    S s;
    int *p = &s.x;
    p[3] = 0; // <- reaches past 'x' into the rest of 's'
}
```

After:
```cpp
struct S { int x; int y; };
void f() {
    S s;
    s.y = 0;
}
```

## Related checkers

- [arrayIndexOutOfBounds.md](arrayIndexOutOfBounds.md) - the more general out-of-bounds-access check,
  for a plain array rather than the address of a single struct member.
