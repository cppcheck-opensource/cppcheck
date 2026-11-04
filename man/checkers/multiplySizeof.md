# multiplySizeof and divideSizeof

**Message**: Multiplying sizeof() with sizeof() indicates a logic error.<br/>
**Category**: Correctness<br/>
**Severity**: Warning (inconclusive)<br/>
**Language**: C/C++

## Description

- `multiplySizeof`: `sizeof(a) * sizeof(b)` - multiplying two `sizeof` results together almost never
  makes sense dimensionally (the result would be "bytes squared").
- `divideSizeof`: dividing the result of `sizeof()` on a pointer type by another `sizeof()` - since
  `sizeof(pointer)` is the size of the pointer itself, not of the data it points to, this computes a
  meaningless ratio.

Both require `--inconclusive` to be enabled.

## Motivation

Both patterns are dimensionally suspicious: multiplying two byte-counts together, or dividing a
pointer's fixed size by an unrelated element size, essentially never produces the number a program
actually needs. They're the kind of typo (an extra `sizeof`, or a `sizeof` applied to the wrong
variable) that's easy to introduce and easy to miss, since the code still compiles.

## How to fix

Before:
```cpp
void f() {
    int a = 1, b = 1;
    int s = sizeof(a) * sizeof(b); // <- multiplying two sizeof() results
}
```

After:
```cpp
void f() {
    int a = 1, b = 1;
    int s = sizeof(a) * b; // multiply by the count, not another sizeof()
}
```

Before:
```cpp
void f(int *p) {
    int n = 100 / sizeof(p); // <- dividing by the pointer's own size
}
```

After:
```cpp
void f(int *p) {
    int n = 100 / sizeof(*p);
}
```

## Related checkers

- [pointerSize.md](pointerSize.md) - the same "size of pointer, not of data" mistake, specifically
  when used as (or to compute) an argument to `malloc`/`memcpy`/`memset`-family functions.
