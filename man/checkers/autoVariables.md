# autoVariables

**Message**: Address of local auto-variable assigned to a function parameter.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C/C++

## Description

The address of a local variable is assigned to a pointer function *parameter* (directly, through a
struct member, or through an array element reached via that parameter), so the address leaks back to
the caller.

## Motivation

The assignment itself just stores an address - the trouble starts once the function returns and `num`
goes out of scope: the caller is left holding a pointer to memory it doesn't own anymore, and using that
pointer (which is the entire reason it was written out through the parameter) is undefined behaviour.
Because the memory involved is usually still intact for a little while afterwards, this kind of bug
often "works" in testing and then fails unpredictably once something else reuses that memory - which
makes it worth catching at analysis time instead of at runtime.

## How to fix

Before:
```cpp
void foo(int **res) {
    int num = 2;
    *res = &num; // <- autoVariables: 'num' won't exist once foo() returns
}
```

After: return the value itself, or allocate storage that outlives the function.
```cpp
void foo(int *res) {
    int num = 2;
    *res = num;
}
```

## Related checkers

- [danglingLifetime.md](danglingLifetime.md) - the same idea, but for a local variable's address
  escaping into a global/static/member pointer instead of a function parameter.
- [returnDanglingLifetime.md](returnDanglingLifetime.md) - the same idea, but escaping through `return`
  instead of a parameter.
