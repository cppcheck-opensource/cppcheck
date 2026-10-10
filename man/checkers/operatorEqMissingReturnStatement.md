# operatorEqMissingReturnStatement

**Message**: No 'return' statement in non-void function causes undefined behavior.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C++

## Description

An `operator=` reaches the end of its body with no `return` at all, and isn't one of the deliberate
"never returns" cases covered by
[operatorEqShouldBeLeftUnimplemented.md](operatorEqShouldBeLeftUnimplemented.md) - undefined behaviour,
since the (non-`void`) function is expected to produce a value.

## Motivation

Falling off the end of a value-returning function without a `return` is undefined behaviour in C++ in
its own right, per the standard - it doesn't require the caller to go on and use the "returned" value;
merely reaching the closing `}` without having returned anything is already the undefined action.

## How to fix

Add the missing `return *this;`.

Before:
```cpp
class szp
{
  szp &operator =(int *other) {} // <- no return statement
};
```

After:
```cpp
class szp
{
  szp &operator =(int *other) { return *this; }
};
```

## Related checkers

- [operatorEqRetRefThis.md](operatorEqRetRefThis.md) - the general check for `operator=` not returning
  `*this`.
- [operatorEqShouldBeLeftUnimplemented.md](operatorEqShouldBeLeftUnimplemented.md) - the case where
  `operator=` deliberately never returns (for example, always throwing), and should instead be declared
  unimplemented/deleted rather than just missing a `return`.
