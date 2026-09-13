# redundantCondition

**Message**: Redundant condition: The condition 'x != 4' is redundant since 'x == 3' is sufficient.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

Two comparisons on the same variable, joined by `&&`/`||`, where one comparison is already implied by
the other and adds nothing.

## Motivation

If `x == 3` is true, `x != 4` is automatically true too - so `(x == 3) && (x != 4)` behaves exactly like
`x == 3` alone. The extra comparison doesn't change what the code does, only makes it longer and harder
to read, and can suggest a stricter check was intended than what's actually enforced.

This check may need `--check-level=exhaustive` to see every case.

## How to fix

Before:
```cpp
void f(int x, int a) {
    if ((x==3) && (x!=4)) // <- 'x != 4' adds nothing once 'x == 3' is true
        a++;
}
```

After:
```cpp
void f(int x, int a) {
    if (x==3)
        a++;
}
```

## Related checkers

- [incorrectLogicOperator.md](incorrectLogicOperator.md) - the same style of two-comparisons-on-one-variable
  analysis, but for when the combination is always entirely true or entirely false.
