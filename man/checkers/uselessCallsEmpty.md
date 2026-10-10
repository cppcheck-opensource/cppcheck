# uselessCallsEmpty

**Message**: Ineffective call of function 'empty()'. Did you intend to call 'clear()' instead?<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C++

## Description

`.empty()` is called and its result is thrown away - since `.empty()` has no side effect, this almost
certainly should have been `.clear()`.

## Motivation

`.empty()` only reports whether a container has any elements; it never removes anything. Calling it and
discarding the result does nothing at all, which is a strong sign the intended call was `.clear()`
(which does have the effect the code was presumably trying to achieve).

## How to fix

Before:
```cpp
#include <vector>
bool foo(std::vector<int>& v) {
    v.empty(); // <- result discarded, has no effect
    return v.empty();
}
```

After:
```cpp
#include <vector>
bool foo(std::vector<int>& v) {
    return v.empty();
}
```
