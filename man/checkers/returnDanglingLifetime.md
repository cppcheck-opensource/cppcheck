# returnDanglingLifetime

**Message**: Returning pointer to local variable 'num' that will be invalid when returning.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C/C++

## Description

A function returns a pointer, iterator, or a lambda that captures one, which refers to a local
variable, array, or container.

## Motivation

Using a pointer after the local variable it points to has gone out of scope is undefined behaviour.
Because the memory involved is usually still intact for a little while afterwards, this kind of bug
often "works" in testing and then fails unpredictably once something else reuses that memory - which
makes it worth catching at analysis time instead of at runtime.

## How to fix

Before:
```cpp
int* foo() {
    int num = 2;
    return &num; // <- returnDanglingLifetime
}
```

After:
```cpp
int* foo() {
    static int num = 2;
    return &num;
}
```

## Related checkers

- [returnReference.md](returnReference.md) - the same idea, but for a function whose declared return
  type is a reference rather than a pointer.
- [autoVariables.md](autoVariables.md) - the same idea, but escaping through a function parameter
  instead of `return`.
