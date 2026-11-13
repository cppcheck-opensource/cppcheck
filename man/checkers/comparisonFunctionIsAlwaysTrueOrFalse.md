# comparisonFunctionIsAlwaysTrueOrFalse

**Message**: Comparison of two identical variables with isless(x,x) always evaluates to false.<br/>
**Category**: Code Quality<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

One of the C99 comparison macros (`isgreater`, `isless`, `islessgreater`, `isgreaterequal`,
`islessequal`) is called with the exact same variable as both arguments, which always evaluates to the
same result.

## Motivation

Comparing a value against itself with a strict-ordering macro always produces the same, fixed answer -
so the call doesn't test anything, and is almost always a copy-paste mistake where the second argument
should have been a different variable.

## How to fix

Before:
```cpp
#include <cmath>
bool f(int x) {
   return isless(x,x); // <- always false
}
```

After:
```cpp
#include <cmath>
bool f(int x, int y) {
   return isless(x,y);
}
```

