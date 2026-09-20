# invalidLifetime

**Message**: Using object that is out of scope.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C/C++

## Description

A pointer (or a lambda capturing one) that refers to a variable is used after that variable's scope has
already ended - for example, it was only set inside an `if` block, and is used after the block closes.

## Motivation

A variable declared inside a nested block stops existing once that block ends, even though the pointer
that referred to it is still sitting in an outer scope. Using that pointer afterwards is undefined
behaviour, and it's easy to overlook because the pointer variable itself is still very much "in scope."

## How to fix

Before:
```cpp
void f(bool cond) {
    int* p;
    if (cond) {
        int x = 1;
        p = &x;
    }
    *p = 2; // <- invalidLifetime: 'x' is out of scope here on the 'cond' path
}
```

After:
```cpp
void f(bool cond) {
    int local = 1;
    int* p = &local;
    if (cond) {
        *p = 1;
    }
    *p = 2;
}
```

## Related checkers

- [danglingTemporaryLifetime.md](danglingTemporaryLifetime.md) - the same idea, but for a pointer or
  iterator into a temporary object instead of a named local variable.
