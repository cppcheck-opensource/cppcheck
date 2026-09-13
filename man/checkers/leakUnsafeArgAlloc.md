# leakUnsafeArgAlloc

**Message**: Unsafe allocation. If shared_ptr() throws, memory could be leaked. Use make_shared<T>() instead.<br/>
**Category**: Correctness<br/>
**Severity**: Warning (inconclusive)<br/>
**Language**: C++

## Description

A call constructs a `shared_ptr`/`unique_ptr` around a `new` expression as one argument, while another
argument is a function call that might throw - if that other call throws before the smart pointer is
constructed, the raw `new` can leak (a classic pre-C++17 evaluation-order hazard). This is only reported
with `--inconclusive`, since cppcheck can't tell whether the other argument can actually throw.

## Motivation

Before C++17, the order in which a function call's arguments (and the work needed to construct them)
run relative to each other was unspecified. If `new int(42)` runs, and then the *other* argument's
function call throws before `shared_ptr<int>(...)` gets to wrap it, the raw pointer from `new` is never
adopted by anything and leaks when the exception propagates. `make_shared`/`make_unique` avoid the
hazard entirely, since there's no separate raw pointer that can be orphaned this way.

## How to fix

Before:
```cpp
void g();
void f(shared_ptr<int> p, int x);
void x() {
    f(shared_ptr<int>(new int(42)), g()); // <- leaks if g() throws
}
```

After:
```cpp
void g();
void f(shared_ptr<int> p, int x);
void x() {
    f(make_shared<int>(42), g());
}
```

