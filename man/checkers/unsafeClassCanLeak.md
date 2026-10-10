# unsafeClassCanLeak

**Message**: Class 'A' is unsafe, 'A::b' can leak by wrong usage.<br/>
**Category**: Correctness<br/>
**Severity**: Style<br/>
**Language**: C++

## Description

A class allocates a member (typically in its constructor) but has no visible destructor logic that
frees it - so every place that creates one of these objects has to remember to clean it up manually,
which is easy to get wrong.

## Motivation

A class that owns an allocation but doesn't free it in its own destructor pushes the cleanup
responsibility out onto every single place that uses the class - if even one of those places forgets, or
an exception skips the manual cleanup, the allocation leaks. Freeing what the class owns, inside its own
destructor, makes the class safe to use without every caller having to think about it.

## How to fix

Before:
```cpp
class A {
    int *b;
public:
    A() { b = new int; } // <- no destructor frees 'b'
};
```

After:
```cpp
class A {
    int *b;
public:
    A() { b = new int; }
    ~A() { delete b; }
};
```

