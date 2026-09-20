# returnStdMoveLocal

**Message**: Using std::move for returning object by-value from function will affect copy elision optimization.<br/>
**Category**: Code Quality<br/>
**Severity**: Performance<br/>
**Language**: C++ only

## Description

`return std::move(x);` is used for a local variable or temporary that's returned by value - this
defeats the compiler's copy elision (RVO/NRVO), which would otherwise avoid the copy entirely without
`std::move`.

## Motivation

When a local variable is returned by value, the compiler is allowed (and, since C++17 in many cases,
required) to construct it directly in the caller's storage, skipping any copy or move entirely. Wrapping
the return in `std::move()` defeats this optimization by forcing a move instead, which is strictly worse
than the elision the compiler would have done on its own.

## How to fix

Before:
```cpp
struct A{};
A f() {
    A var;
    return std::move(var); // <- defeats copy elision
}
```

After:
```cpp
struct A{};
A f() {
    A var;
    return var;
}
```

## Related checkers

- [useStandardLibrary.md](useStandardLibrary.md) - an unrelated performance suggestion in the same
  checker: replacing a hand-written copy loop with a standard library call.
