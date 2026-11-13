# operatorEqShouldBeLeftUnimplemented

**Message**: 'operator=' should either return reference to 'this' instance or be declared private and left unimplemented.<br/>
**Category**: Correctness<br/>
**Severity**: Style<br/>
**Language**: C++

## Description

An `operator=` with no `return` statement, whose entire body is a `throw` (or a call to a function that
never returns) - a common way to try to make an assignment operator uncallable, but the idiomatic way is
to declare it `private`/`= delete` instead of giving it a body that never returns.

## Motivation

Giving `operator=` a body that always throws is a roundabout, easy-to-misread way of saying "this class
can't be assigned" - it still compiles as if it were a normal, callable assignment operator, and the
"can't be assigned" part is only enforced at runtime, when it's too late to catch at compile time. The
idiomatic ways (`= delete`, or a private declaration with no definition) reject the attempt to assign at
compile time instead.

## How to fix

Declare the assignment operator `= delete` (or `private` with no body) rather than giving it a body
that always throws.

Before:
```cpp
#include <cstdlib>
#include <stdexcept>
class A {
public:
    A & operator=(const A &a) {
        rand();
        throw std::exception(); // <- always throws instead of assigning
    }
};
```

After:
```cpp
class A {
public:
    A & operator=(const A &a) = delete;
};
```

## Related checkers

- [operatorEqRetRefThis.md](operatorEqRetRefThis.md) - the general check for `operator=` not returning
  `*this`, of which this is a specific variant.
- [operatorEqMissingReturnStatement.md](operatorEqMissingReturnStatement.md) - the sibling case where
  `operator=` reaches the end of its body with no return, but isn't one of these deliberate
  "never returns" cases.
