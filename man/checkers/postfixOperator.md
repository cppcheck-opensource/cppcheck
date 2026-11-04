# postfixOperator

**Message**: Prefer prefix ++/-- operators for non-primitive types.<br/>
**Category**: Performance<br/>
**Severity**: Performance<br/>
**Language**: C++ only

## Description

This checker suggests using the prefix `++`/`--` operator instead of postfix on a variable (or member
variable) of a non-primitive type - a class, struct or union instance, or a type whose name ends in
`iterator`, `const_iterator`, `reverse_iterator` or `const_reverse_iterator` - whenever the old value
produced by the postfix operator is not actually used, for example as a whole statement (`k++;`) or in
a loop's increment clause (`for (...; ...; i++)`).

It does not warn when the postfix result is genuinely used, for example when passed as a function
argument (`foo(a++)`), since switching to prefix there would change the program's behavior, not just
its performance.

This checker only runs when the `performance` severity is enabled, and only applies to C++ (there are
no classes in C).

## Motivation

For a non-primitive type, the postfix operator typically has to make a copy of the object's previous
value before modifying it, so that the old value can be returned - even when nothing uses that old
value. The prefix operator does not need this copy. Built-in types like `int` and pointers don't pay
for this copy, which is why the checker never warns about those.

## How to fix

Use the prefix operator when the previous value isn't needed.

Before:
```cpp
class BigNumber { /* ... */ };

void f(BigNumber& n) {
    n++; // <- postfixOperator: the returned old value is discarded
}
```

After:
```cpp
class BigNumber { /* ... */ };

void f(BigNumber& n) {
    ++n;
}
```

