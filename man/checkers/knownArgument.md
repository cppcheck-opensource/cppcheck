# knownArgument and knownArgumentHiddenVariableExpression

**Message**: Argument 'x-x' to function 'func' is always 0. It does not matter what value 'x' has.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

- `knownArgument`: a function/constructor argument is an expression whose value cppcheck can already
  compute at compile time, regardless of what a variable inside it holds - so passing that variable in
  was pointless.
- `knownArgumentHiddenVariableExpression`: same idea, but specifically when a constant part of the
  expression (`&& false`, `* 0`, `|| true`) completely masks a variable that looks like it should have
  mattered - a common sign of a stray/leftover condition.

## Motivation

When an argument's value doesn't actually depend on a variable that's visibly part of its expression,
that's either dead code (the variable can be removed) or a sign the expression doesn't do what its
author expected - especially in the "hidden variable" case, where a constant operator silently cancels
out something that looks meaningful.

## How to fix

Before:
```cpp
void g(int);
void f(int x) {
   g((x & 0x01) >> 7); // <- always 0, no matter what 'x' is
}
```

After:
```cpp
void g(int);
void f(int x) {
   g(x >> 7);
}
```

Before:
```cpp
void dostuff(int);
void f(int x) {
    dostuff(x * 0); // <- 'x' never actually matters here
}
```

After:
```cpp
void dostuff(int);
void f(int x) {
    dostuff(x);
}
```
