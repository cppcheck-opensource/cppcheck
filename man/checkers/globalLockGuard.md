# globalLockGuard

**Message**: Lock guard is defined globally. Lock guards are intended to be local. A global lock guard could lead to a deadlock since it won't unlock until the end of the program.<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C++

## Description

A `std::lock_guard` (or similar RAII lock wrapper) is declared `static` or at namespace/global scope -
it's then held for the entire lifetime of the program, since its destructor (which releases the lock)
never runs until the program exits, defeating the purpose of the lock and risking a permanent deadlock.

## Motivation

A `lock_guard`'s whole purpose is to release the lock automatically when it goes out of scope. Giving
it static storage duration means it never goes out of scope until the program ends - the lock is taken
once and never released, so any other code that later tries to lock the same mutex blocks forever.

## How to fix

Before:
```cpp
#include <mutex>
void f() {
    static std::mutex m;
    static std::lock_guard<std::mutex> g(m); // <- never unlocked until program exit
}
```

After:
```cpp
#include <mutex>
void f() {
    static std::mutex m;
    std::lock_guard<std::mutex> g(m);
}
```

## Related checkers

- [localMutex.md](localMutex.md) - the opposite mistake: a mutex and its lock declared in the same,
  too-narrow scope, so the lock has no effect.
