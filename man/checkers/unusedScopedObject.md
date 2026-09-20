# unusedScopedObject

**Message**: Instance of 'x' object is destroyed immediately.<br/>
**Category**: Correctness<br/>
**Severity**: Style<br/>
**Language**: C++

## Description

An object is constructed as a temporary (`Lock(mutex);` instead of `Lock lock(mutex);`) and destroyed
again immediately at the end of the same statement - if the class's constructor/destructor pair matters
for its side effects (as with a scope guard), this defeats the purpose, since the "lock" is released
before the very next statement runs.

## Motivation

RAII types like lock guards, scoped timers, or transaction guards rely on their destructor running at
the end of a *scope*, not at the end of the single statement that constructed them. Forgetting to give
the object a name accidentally destroys it right away, silently disabling whatever protection it was
supposed to provide for the following code.

## How to fix

Before:
```cpp
#include <iostream>
class Lock {
public:
    Lock(int i) { std::cout << "Lock " << i << std::endl; }
    ~Lock() { std::cout << "~Lock" << std::endl; }
};
int main() {
    Lock(123); // <- constructed and destroyed on this line alone
    std::cout << "hello" << std::endl;
    return 0;
}
```

After:
```cpp
#include <iostream>
class Lock {
public:
    explicit Lock(int i) { std::cout << "Lock " << i << std::endl; }
    ~Lock() { std::cout << "~Lock" << std::endl; }
};
int main() {
    Lock lock(123);
    std::cout << "hello" << std::endl;
    return 0;
}
```
