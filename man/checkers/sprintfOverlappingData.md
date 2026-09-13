# sprintfOverlappingData

**Message**: Undefined behavior: Variable is used as parameter and destination in s[n]printf().<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C/C++

## Description

The same variable is used as both the destination and a source argument of `sprintf`/`snprintf`/
`swprintf`, which is undefined behaviour.

## Motivation

`sprintf()` and its relatives may write to the destination buffer while still reading from it if a
source argument aliases the destination - the C standard leaves this undefined, so the actual result
(correct output, garbled output, or a crash) depends on the specific library implementation.

## How to fix

Before:
```cpp
#include <cstdio>
void foo() {
    char buf[100];
    sprintf(buf, "%s", buf); // <- source and destination overlap
}
```

After:
```cpp
#include <cstdio>
void foo() {
    char buf[100] = "hi";
    char tmp[100];
    sprintf(tmp, "%s", buf); // write to a different buffer
}
```

## Related checkers

- [overlappingStrcmp.md](overlappingStrcmp.md) - an unrelated string-function misuse in the same
  checker, about a redundant/contradictory pair of `strcmp()` checks.
