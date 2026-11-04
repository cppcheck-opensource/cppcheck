# throwInEntryPoint

**Message**: Unhandled exception thrown in function that is an entry point.<br/>
**Category**: Correctness<br/>
**Severity**: Error<br/>
**Language**: C++ only

## Description

An exception can escape a recognized program entry point (`main`, or others such as `_init`/`_fini`
when the matching library configuration is loaded).

## Motivation

An exception that escapes `main()` (or another entry point) is not caught by anything, so the C++
runtime calls `std::terminate()` - the program aborts, typically without the cleanup a caught,
handled exception would have allowed.

## How to fix

Before:
```cpp
void doWork();
int main() {
    doWork(); // <- if this throws, nothing will catch it
    return 0;
}
```

After:
```cpp
#include <exception>
void doWork();
int main() {
    try {
        doWork();
    } catch (const std::exception&) {
        return 1;
    }
    return 0;
}
```

## Related checkers

- [throwInNoexceptFunction.md](throwInNoexceptFunction.md) - the same underlying problem, for a
  function explicitly marked `noexcept` instead of a recognized entry point.
