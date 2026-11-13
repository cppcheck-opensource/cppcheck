# exceptThrowInDestructor

**Message**: Class X is not safe, destructor throws exception<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C++ only

## Description

A destructor throws an exception.

## Motivation

Destructors are implicitly `noexcept` since C++11, and even before that, throwing from a destructor
during stack unwinding (while another exception is already propagating) calls `std::terminate()`
immediately, aborting the program without normal cleanup.

## How to fix

Before:
```cpp
class Resource {
public:
    ~Resource() {
        throw 1; // <- destructors are expected not to throw
    }
};
```

After:
```cpp
class Resource {
public:
    ~Resource() {
    }
};
```

## Related checkers

- [throwInNoexceptFunction.md](throwInNoexceptFunction.md) - the same underlying problem, for any
  function explicitly marked `noexcept` rather than specifically a destructor.
