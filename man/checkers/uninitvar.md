# uninitvar and legacyUninitvar

**Message**: Uninitialized variable: x<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error/Warning<br/>
**Language**: C/C++

## Description

A local variable, pointer, or struct member is read (or its uninitialized value is passed along)
before any assignment reaches that point:

- `uninitvar`: the main check.
- `legacyUninitvar`: an older, separate analysis that catches some patterns the main `uninitvar` check
  doesn't (and vice versa) - both run, so the same kind of bug can be reported under either ID depending
  on the exact code shape.

## Motivation

Reading a variable before it has been given a value is undefined behaviour in C/C++: the variable's
contents are whatever bytes happened to already be in that memory, which is unpredictable and can
differ between runs, builds, or optimization levels. This is one of the most common sources of subtle,
hard-to-reproduce bugs.

## How to fix

Before:
```cpp
#include <cstdio>
void f() {
    int x;
    printf("%d", x); // <- uninitvar
}
```

After:
```cpp
#include <cstdio>
void f() {
    int x = 0;
    printf("%d", x);
}
```

## False positives to be aware of

- **Passing a member-access expression as an argument to an unresolved, all-uppercase macro-like call**
  can be wrongly flagged, since cppcheck can't tell whether the macro actually evaluates its argument
  at runtime (many such macros, like size- or offset-computing ones, don't):
  ```cpp
  void f() {
      struct SData * s;
      TYPEOF(s->status); // flagged as reading uninitialized 's', even if TYPEOF never evaluates it
  }
  ```
- **A cast applied to `&variable` can be misread as reading the variable itself**, when it's actually
  ambiguous whether `&` means "address of" (which never requires the variable to hold a value) or a
  bitwise AND:
  ```cpp
  int main() {
      int done;
      dostuff(1, (AuPointer) &done); // flagged, even though '&done' alone doesn't read 'done'
  }
  ```
- **Casting a variable of a type cppcheck doesn't recognize to a pointer type can be wrongly treated as
  reading it**, particularly in C code using an opaque/typedef'd type:
  ```c
  void f() {
      DES_cblock d;     // unknown type
      char *dp;
      dp = (char *)d;   // flagged as reading uninitialized 'd', even though the type is opaque to cppcheck
  }
  ```

## Related checkers

- [uninitdata.md](uninitdata.md) - the same idea, for memory obtained from an allocation function
  rather than a plain variable.
- [uninitStructMember.md](uninitStructMember.md) - the same idea, narrowed to one specific struct
  member.
- [ctuuninitvar.md](ctuuninitvar.md) - the same idea, found by whole-program analysis across function
  calls.
