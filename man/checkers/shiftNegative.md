# shiftNegative

**Message**: Shifting by a negative value is undefined behaviour<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C/C++

## Description

A bitwise shift (`<<`/`>>`) is done by a negative amount, which is undefined behaviour.

## Motivation

The C/C++ standards leave the result of shifting by a negative amount undefined - the program's actual
behaviour is not guaranteed and can change with the compiler, optimization level, or platform.

## How to fix

Before:
```cpp
void foo() {
   int a; a = 123;
   (void)(a << -1); // <- shifting by a negative amount
}
```

After:
```cpp
void foo() {
   int a; a = 123;
   (void)(a << 1);
}
```

## Related checkers

- [shiftNegativeLHS.md](shiftNegativeLHS.md) - the same kind of undefined behaviour, but for shifting a
  negative value rather than shifting by a negative amount.
