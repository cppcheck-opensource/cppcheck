# localMutex

**Message**: The lock is ineffective because the mutex is locked at the same scope as the mutex itself.<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C++

## Description

A `std::mutex` and the `lock_guard`/`.lock()` call that locks it are declared in the very same scope -
the mutex is a fresh, unshared object every time that scope is entered, so nothing else can ever be
contending for it and the lock has no effect.

## Motivation

A mutex only protects shared data if the same mutex instance is reachable from every place that might
access that data concurrently. A mutex that's a local variable, locked in the same function it's
declared in, is a brand-new object on every call - no other thread (or even another call to the same
function) can ever see or contend for that exact instance, so the "protection" is illusory.

## How to fix

Before:
```cpp
#include <mutex>
void f() {
    std::mutex m;
    std::lock_guard<std::mutex> g(m); // <- 'm' is a fresh, private mutex every call
}
```

After:
```cpp
#include <mutex>
std::mutex m;
void f() {
    std::lock_guard<std::mutex> g(m);
}
```

## Related checkers

- [globalLockGuard.md](globalLockGuard.md) - the opposite mistake: a lock guard given static/global
  storage, so it never releases the lock.
