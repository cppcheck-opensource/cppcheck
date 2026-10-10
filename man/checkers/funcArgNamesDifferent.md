# funcArgNamesDifferent and funcArgNamesDifferentUnnamed

**Message**: Function 'f' argument 1 names different: declaration 'a' definition 'b'.<br/>
**Category**: Code Quality<br/>
**Severity**: Style (Inconclusive)<br/>
**Language**: C/C++

## Description

A function is declared in one place (for example a header) and defined in another, and an argument has
a different name between the two (`funcArgNamesDifferent`), or is named in one but not the other
(`funcArgNamesDifferentUnnamed`).

## Motivation

Readers of the declaration alone can be misled about what an argument means if the definition uses a
different (or more descriptive) name - the declaration is often the only thing visible from a header,
while the actual logic and its more meaningful names live in the definition.

## How to fix

Before:
```cpp
void func2(int a, int b, int c);
void func2(int A, int B, int C) { } // <- names don't match the declaration
```

After:
```cpp
void func2(int a, int b, int c);
void func2(int a, int b, int c) { }
```

Before:
```cpp
void f(int a);
void f(int) {} // <- the definition drops the declared name
```

After:
```cpp
void f(int a);
void f(int a) {}
```

## Related checkers

- [funcArgOrderDifferent.md](funcArgOrderDifferent.md) - the same kind of declaration/definition
  mismatch, but for the order of two arguments that keep their names.
