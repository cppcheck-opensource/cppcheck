# throwInNoexceptFunction

**Message**: Unhandled exception thrown in function declared not to throw exceptions.<br/>
**Category**: Correctness<br/>
**Severity**: Error<br/>
**Language**: C++ only

## Description

A function declared `noexcept`, `throw()`, or with an equivalent `__attribute__((nothrow))`/
`__declspec(nothrow)`, throws (or calls something that throws) anyway.

## Motivation

If an exception escapes a function promised not to throw, `std::terminate()` is called immediately -
the program aborts without any of the normal stack-unwinding cleanup (destructors further up the call
stack are not guaranteed to run).

## How to fix

Before:
```cpp
void f() noexcept {
    throw 1; // <- contradicts the noexcept promise
}
```

After:
```cpp
void f() noexcept {
}
```

## Related checkers

- [exceptThrowInDestructor.md](exceptThrowInDestructor.md) - the same underlying problem, specifically
  for a destructor (implicitly `noexcept`).
- [throwInEntryPoint.md](throwInEntryPoint.md) - the same underlying problem, for a recognized program
  entry point instead of an explicitly `noexcept` function.
