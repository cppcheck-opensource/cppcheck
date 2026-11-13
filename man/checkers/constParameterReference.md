# constParameterReference

**Message**: Parameter 'x' can be declared as reference to const<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C++

## Description

A reference parameter is never used to modify what it refers to, so it could be declared as a reference
to `const`.

## Motivation

A missing `const` hides a guarantee the compiler could otherwise enforce and readers could otherwise
rely on: that the function only reads through the reference, never modifies the caller's object through
it.

## How to fix

Before:
```cpp
#include <vector>
auto foo(std::vector<int>& vec, bool flag) { // <- 'vec' is only read
    std::vector<int> dummy;
    std::vector<int>::iterator iter;
    if (flag)
        iter = vec.begin();
    else {
        dummy.push_back(42);
        iter = dummy.begin();
    }
    return *iter;
}
```

After:
```cpp
#include <vector>
auto foo(const std::vector<int>& vec, bool flag) {
    std::vector<int> dummy;
    std::vector<int>::iterator iter;
    if (flag)
        iter = dummy.begin();
    else {
        dummy.push_back(42);
        iter = dummy.begin();
    }
    return *iter;
}
```

## Related checkers

- [constParameter.md](constParameter.md) - the array-parameter equivalent.
- [constParameterPointer.md](constParameterPointer.md) - the pointer-parameter equivalent.
- [constVariableReference.md](constVariableReference.md) - the same idea, for a local reference
  variable instead of a parameter.
- [constParameterCallback.md](constParameterCallback.md) - the same idea, but for a parameter of a
  function used as a callback.
