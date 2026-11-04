# knownPointerToBool

**Message**: Pointer expression 'p' converted to bool is always true.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

A pointer already known to be non-null (its address was just taken, or it was just checked) is
converted to `bool` - the result can only ever be `true`.

## Motivation

Converting a pointer that's already known to be non-null to `bool` doesn't test anything - it always
evaluates to `true`, so the code reads as if it's checking something that it isn't.

## How to fix

Before:
```cpp
void g(bool);
void f() {
    int i = 5;
    int* p = &i;
    g(p); // <- 'p' can't be null here, so this is always true
}
```

After:
```cpp
void g(bool);
void f(int* p) {
    g(p);
}
```

