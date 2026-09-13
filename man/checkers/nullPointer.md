# nullPointer

**Message**: Null pointer dereference<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C/C++

## Description

A pointer is definitely, or very likely, null at the point it is dereferenced - for example, it was
just assigned `0`/`nullptr`, or a variable known to be null was passed in.

## Motivation

Dereferencing a null pointer is undefined behaviour and one of the most common causes of crashes in
C/C++ programs. cppcheck reports this only when it can actually determine the pointer is null (or very
likely null); it does not attempt to trace every possible path a pointer's value could have taken to
get there, so the absence of a warning does not by itself mean a pointer can never be null at that
point.

## How to fix

Before:
```cpp
void f() {
    int *p = 0;
    *p = 1; // <- nullPointer
}
```

After:
```cpp
void f() {
    int x = 0;
    int *p = &x;
    *p = 1;
}
```

## False positives to be aware of

- **A false positive is possible when a guard condition depends on a different parameter than the one
  being dereferenced.** If a function only dereferences one pointer when another argument has a
  specific value, cppcheck does not always verify that a given call site actually triggers that
  value, and can warn even when the guard makes the dereference unreachable for that call:
  ```cpp
  void f(int* p, const int* q) {
      if (*q == -1)
          *p = 0;
  }
  void g() {
      int x = -2;
      f(nullptr, &x); // reported as a possible null dereference of 'p', even though *q==-1 is false here
  }
  ```

## Related checkers

- [nullPointerRedundantCheck.md](nullPointerRedundantCheck.md) - the same idea, but where a null check on the same pointer exists elsewhere and either it or the dereference is misplaced.
- [nullPointerDefaultArg.md](nullPointerDefaultArg.md) - the same idea for a pointer parameter that defaults to null.
- [nullPointerOutOfMemory.md](nullPointerOutOfMemory.md) - the same idea for a pointer that came from an allocation function that can fail.
- [nullPointerArithmetic.md](nullPointerArithmetic.md) - the same idea for pointer arithmetic instead of a direct dereference.
- [ctunullpointer.md](ctunullpointer.md) - the same idea, found by whole-program analysis across function calls.
