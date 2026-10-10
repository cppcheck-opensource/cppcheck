# arrayIndexOutOfBounds and arrayIndexOutOfBoundsCond

**Message**: Array 'a[10]' accessed at index 20, which is out of bounds.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error/Warning<br/>
**Language**: C/C++

## Description

An array is indexed with a value outside its bounds:

- `arrayIndexOutOfBounds`: cppcheck knows for certain the index is outside the array's bounds.
- `arrayIndexOutOfBoundsCond`: the out-of-bounds index only holds on one branch of a condition checked
  on the index variable elsewhere in the code - so either that condition is redundant, or this access
  is a bug.

## Motivation

Reading or writing outside the bounds of an array is undefined behaviour: at best the program crashes
immediately, at worst it silently corrupts nearby memory and fails much later, or in a different run,
in a way that's very hard to trace back to the actual cause.

## How to fix

Before:
```cpp
void f() {
    int a[10];
    a[20] = 0; // <- arrayIndexOutOfBounds
}
```

After:
```cpp
void f() {
    int a[20];
    a[19] = 0;
}
```

## Related checkers

- [pointerOutOfBounds.md](pointerOutOfBounds.md) - the pointer-arithmetic equivalent of this check.
- [ctuArrayIndex.md](ctuArrayIndex.md) - the same idea, found by cppcheck's whole-program analysis
  across function calls.
- [negativeIndex.md](negativeIndex.md) - the same idea, specifically for a negative index.
- [objectIndex.md](objectIndex.md) - a related out-of-bounds access, through the address of a specific
  struct member instead of an array.
