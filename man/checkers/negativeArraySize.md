# negativeArraySize and negativeMemoryAllocationSize

**Message**: Declaration of array 'a' with negative size is undefined behaviour<br/>
**Category**: Correctness/Undefined Behaviour<br/>
**Severity**: Error/Warning<br/>
**Language**: C/C++

## Description

Something that determines the size of an array-like allocation is known to be negative:

- `negativeArraySize`: a variable-length array is declared with a size cppcheck knows is negative.
- `negativeMemoryAllocationSize`: a `new[]` expression is given a size cppcheck knows is negative.

## Motivation

- `negativeArraySize`: a variable-length array's size, once converted to the unsigned type it's ultimately
  represented as, becomes an enormous positive number - so instead of failing cleanly, the declaration
  either crashes outright or succeeds with a wildly wrong, huge size. This is undefined behaviour.
- `negativeMemoryAllocationSize`: in C++11 and later, `new[]` is specifically required to detect this and
  throw `std::bad_array_new_length` instead - so this specific case is a well-defined, catchable error
  rather than undefined behaviour, but it's still a bug worth flagging: an uncaught exception terminates
  the program, and any earlier C++ standard leaves the size negotiation undefined instead.

## How to fix

Before:
```cpp
void f() {
    int n = -1;
    int a[n]; // <- negativeArraySize
}
```

After:
```cpp
void f() {
    int n = 1;
    int a[n];
}
```

Before:
```cpp
void f() {
    int n = -1;
    int *p = new int[n]; // <- negativeMemoryAllocationSize
}
```

After:
```cpp
void f() {
    int n = 1;
    int *p = new int[n];
    p[0] = 0;
    delete[] p;
}
```

