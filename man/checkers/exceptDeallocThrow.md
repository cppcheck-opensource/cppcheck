# exceptDeallocThrow

**Message**: Exception thrown in invalid state, 'p' points at deallocated memory.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Warning<br/>
**Language**: C++ only

## Description

A global/static pointer is `delete`d and an exception can then be thrown before the pointer is given a
new value, leaving a dangling pointer for any surviving code to use.

## Motivation

If an exception is thrown between freeing a pointer and resetting it, any exception handler (or later
code, if the exception is caught and execution continues) that touches the same global/static pointer
sees a dangling value - a use-after-free that's easy to miss because the code "looks" like it cleans up
properly, just not in a safe order.

## How to fix

Before:
```cpp
static int* p = nullptr;
void f(bool someCondition) {
    delete p;
    if (someCondition)
        throw 1; // <- 'p' is left dangling if this throws
    p = nullptr;
}
```

After:
```cpp
static int* p = nullptr;
void f(bool someCondition) {
    delete p;
    p = nullptr;
    if (someCondition)
        throw 1;
}
```
