# clarifyStatement

**Message**: In expression like '*A++' the result of '*' is unused. Did you intend to write '(*A)++;'?<br/>
**Category**: Code Quality<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

A statement like `*p++;` looks like it dereferences the incremented pointer, but `++` binds tighter
than the dereference - the pointer is incremented and the dereferenced (old) value is simply discarded.

## Motivation

`*p++` is parsed as `*(p++)`: the pointer itself is incremented, and the value that was pointed to
before the increment is read and then thrown away, since the statement doesn't do anything with it. A
reader skimming the code can easily assume the intent was to modify what the pointer points to
(`(*p)++`), which is a different operation entirely.

## How to fix

Add parentheses to make the intended operation explicit.

Before:
```cpp
char* f(char* c) {
    *c++; // <- increments 'c' and discards the old *c, doesn't touch what 'c' points to
    return c;
}
```

After:
```cpp
char* f(char* c) {
    (*c)++;
    return c;
}
```

## Related checkers

- [clarifyCalculation.md](clarifyCalculation.md) - a different easy-to-misread-precedence pitfall, about
  a calculation next to `?` rather than `*p++`.
