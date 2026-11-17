# missingForLoopIncrement

**Message**: The loop variable 'varname' is not updated in this for-loop, so the loop might never terminate.<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C, C++

## Description

A classic `for` loop's control variable is never advanced in the step clause or the loop body, so the
loop condition may never become false.

## Motivation

Forgetting to increment an iterator or index is a common bug that leads to an infinite loop. Compilers
often do not warn about an empty for-loop step when the programmer intended to update the variable in
the body but never did.

## How to fix

Before:
```cpp
#include <map>
void f(std::map<int, int> &theMap) {
    for (auto it = theMap.begin(); it != theMap.end();) {
        (void)*it; // <- missingForLoopIncrement: 'it' is never updated
    }
}
```

After: increment in the for-header or in the body.
```cpp
#include <map>
void f(std::map<int, int> &theMap) {
    for (auto it = theMap.begin(); it != theMap.end(); ++it) {
        (void)*it;
    }
}
```
