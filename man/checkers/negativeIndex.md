# negativeIndex

**Message**: Array index -11 is out of bounds.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error/Warning<br/>
**Language**: C/C++

## Description

An array is indexed with a value cppcheck knows is negative.

## Motivation

Indexing an array with a negative value reads or writes memory before the start of the array, which is
undefined behaviour - it can silently corrupt unrelated data instead of crashing, making the actual
cause hard to trace back to.

## How to fix

Before:
```cpp
void f(int i) {
    int a[10];
    if (i == -1)
        a[i] = 0; // <- negative index
}
```

After:
```cpp
void f(int i) {
    int a[10];
    if (i >= 0 && i < 10)
        a[i] = 0;
}
```

## Related checkers

- [arrayIndexOutOfBounds.md](arrayIndexOutOfBounds.md) - the general out-of-bounds-index check this
  one complements, for indices known to be too large rather than negative.
