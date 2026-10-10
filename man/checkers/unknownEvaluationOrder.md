# unknownEvaluationOrder

**Message**: Expression 'x = x++;' depends on order of evaluation of side effects<br/>
**Category**: Undefined Behaviour/Portability<br/>
**Severity**: Error/Portability<br/>
**Language**: C/C++

## Description

An expression reads and modifies the same variable more than once with no defined order between the
two (for example `x = x++;` or `x++, x++`). This is reported at two different severities because the
language rules genuinely differ between the two shapes:

- **Error** (for example `x = x++;`): the read and the modification aren't just unordered, they actively
  conflict - this is undefined behaviour in both C and C++.
- **Portability**: the specific expression shape is one where C++17 tightened the sequencing rules
  enough that the order is merely *unspecified* (one of a few valid outcomes, not anything-goes) - not
  undefined behaviour under C++17, but still worth flagging since older compilers/standards, or C, may
  treat the same code as undefined instead.

## Motivation

When the same variable is both read and modified more than once in one expression with no sequencing
between the two, different compilers (or the same compiler at different optimization levels) can
legally produce different results. For the `Error` shape this is undefined behaviour outright; for the
`Portability` shape, C++17 guarantees the result is at least one of a small set of valid outcomes, but
which one is still compiler-dependent, so the code isn't portable even though it's not undefined.

## How to fix

Before:
```cpp
int dostuff();
void f() {
  int x = dostuff();
  return x + x++; // <- depends on evaluation order
}
```

After:
```cpp
int dostuff();
int f() {
  int x = dostuff();
  int y = x++;
  return x + y;
}
```

