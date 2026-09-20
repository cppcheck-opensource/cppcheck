# redundantCopy

**Message**: Buffer 'x' is being written before its old content has been used.<br/>
**Category**: Code Quality<br/>
**Severity**: Performance<br/>
**Language**: C/C++

## Description

A buffer is written, then written again before its first content is ever read - the buffer equivalent
of a redundant variable assignment. In current versions of cppcheck, this specific message is not
actually produced for any input: the code path that would emit it exists in the source but isn't
reachable in practice, so don't expect a warning here even for code that matches the pattern below.

## Motivation

Writing to a buffer twice with nothing reading it in between wastes the work of the first write, which
is the real, intended idea behind this check - a buffer written twice (by `memset`/`strcpy`/similar)
with no read in between, including across fall-through `switch` cases. If you're looking for the
closest actively-working equivalent, see [redundantCopyLocalConst.md](redundantCopyLocalConst.md) (for
`const` variables) or [redundantAssignment.md](redundantAssignment.md) (the same idea for a plain
variable instead of a buffer).

## How to fix

Remove or combine the redundant write so the buffer is only written once before it's read:

```cpp
#include <cstring>
void bar();
void f() {
    char a[10];
    memset(a, 0, 10);
    bar();
}
```

## Related checkers

- [redundantAssignment.md](redundantAssignment.md) - the same idea for a plain variable instead of a buffer.
- [redundantCopyLocalConst.md](redundantCopyLocalConst.md) - a different, currently-working redundant-copy check for `const` variables.
