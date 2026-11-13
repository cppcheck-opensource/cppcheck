# danglingLifetime

**Message**: Non-local variable 'p' will use pointer to local variable 'x'.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C/C++

## Description

The address of a local variable/array is assigned to a *global, static, or member* pointer, so it
outlives the local variable it points to.

## Motivation

Using a pointer after the local variable it points to has gone out of scope is undefined behaviour.
Because the memory involved is usually still intact for a little while afterwards, this kind of bug
often "works" in testing and then fails unpredictably once something else reuses that memory - which
makes it worth catching at analysis time instead of at runtime.

## How to fix

Before:
```cpp
int *p;
void f() {
    int x;
    p = &x; // <- danglingLifetime: 'p' outlives 'x'
}
```

After:
```cpp
int *p;
void f() {
    static int x;
    p = &x;
}
```

Alternatively, if `p` is only meant to point at `x` temporarily and is set back to something else before
`x` disappears, cppcheck already recognizes that as safe and won't warn:
```cpp
int *p;
void f() {
    int x;
    p = &x;
    p = nullptr; // 'p' no longer points to 'x' by the time 'x' is destroyed - not flagged
}
```

## Related checkers

- [autoVariables.md](autoVariables.md) - the same idea, but for a local variable's address escaping
  through a function parameter instead of a global/static/member pointer.
- [danglingReference.md](danglingReference.md) - the same idea, for a reference instead of a pointer.
