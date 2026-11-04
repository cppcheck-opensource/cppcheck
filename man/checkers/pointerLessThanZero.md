# pointerLessThanZero and pointerPositive

**Message**: A pointer can not be negative so it is either pointless or an error to check if it is.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

A pointer is compared against `0` with `<` (`pointerLessThanZero`) or `>=` (`pointerPositive`) - a
pointer value can't be negative, so the first can never be true and the second is always true.

## Motivation

Both comparisons are tautological for a pointer, so the branch they guard either never runs or always
runs - not what the code visibly appears to be testing, and usually a sign that `== nullptr`/`!=
nullptr` was intended instead.

## How to fix

Before:
```cpp
void foo(const int* x) {
  if (x < 0) {} // <- can never be true
}
```

After:
```cpp
void foo(const int* x) {
  if (x == nullptr) {}
}
```

Before:
```cpp
void foo(const int* x) {
  if (x >= 0) {} // <- always true
}
```

After:
```cpp
void foo(const int* x) {
  if (x != nullptr) {}
}
```

## Related checkers

- [unsignedLessThanZero.md](unsignedLessThanZero.md) - the same idea, for an unsigned integer compared
  against `0` instead of a pointer.
