# returnTempReference

**Message**: Reference to temporary returned.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C++

## Description

A function declared to return a reference returns a reference to a temporary object.

## Motivation

A temporary object is destroyed at the end of the full expression that created it (with some narrow
lifetime-extension exceptions that don't apply once the reference is returned out of the function).
Returning a reference to one, and then using it, is a use of an already-destroyed object.

## How to fix

Before:
```cpp
int get_value();
const int &get_reference() {
    const int &x = get_value(); // binds to a temporary
    return x; // <- returnTempReference
}
```

After:
```cpp
int get_value();
int get_reference() {
    return get_value();
}
```

## Related checkers

- [returnReference.md](returnReference.md) - the same idea, but returning a reference to a named local
  variable instead of a temporary.
- [danglingTempReference.md](danglingTempReference.md) - the same underlying temporary-lifetime
  mistake, used within the same function rather than returned from it.
