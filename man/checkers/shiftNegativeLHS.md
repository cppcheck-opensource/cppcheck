# shiftNegativeLHS

**Message**: Shifting a negative value is technically undefined behaviour<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Portability<br/>
**Language**: C/C++

## Description

The value being shifted (rather than the shift amount) is negative, which is technically undefined
behaviour, even though many compilers implement it as a predictable sign-preserving shift.

## Motivation

The C/C++ standards leave the result of shifting a negative value undefined, even though in practice
most compilers implement a predictable arithmetic (sign-preserving) shift - code relying on that isn't
guaranteed to behave the same way on every compiler or platform.

## How to fix

Before:
```cpp
void foo() {
   int x;
   x = (-10+2) << 3; // <- shifting a negative value
}
```

After:
```cpp
void foo() {
   int x;
   x = (10-2) << 3;
}
```

## Related checkers

- [shiftNegative.md](shiftNegative.md) - the same kind of undefined behaviour, but for shifting by a
  negative amount rather than shifting a negative value.
