# nullPointerDefaultArg

**Message**: Possible null pointer dereference if the default parameter value is used.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

A pointer parameter that defaults to `nullptr`/`0` is dereferenced without a null check, so calling the
function with no argument for that parameter crashes.

## Motivation

A default argument value is one every caller is implicitly allowed to rely on - if that default is
null and the function dereferences the parameter unconditionally, calling the function the "normal",
argument-omitted way is itself the bug trigger.

## How to fix

Before:
```cpp
void f(int *p = 0) {
    *p = 1; // <- crashes if called as f()
}
```

After:
```cpp
void f(int *p = 0) {
    if (!p)
        return;
    *p = 1;
}
```

## Related checkers

- [nullPointer.md](nullPointer.md) - the general null-dereference check.
