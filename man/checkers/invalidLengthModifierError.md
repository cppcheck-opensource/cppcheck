# invalidLengthModifierError

**Message**: 'I' in format string (no. 1) is a length modifier and cannot be used without a conversion specifier.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

A length modifier (`h`, `l`, `I64`, ...) appears in the format string with no conversion specifier
after it, so it doesn't actually modify anything.

## Motivation

A length modifier only means something when it's immediately followed by a conversion specifier like
`d` or `u` - on its own it's either a leftover from editing the format string, or a sign that a
specifier letter was accidentally dropped. It also leaves the format string with no valid conversion
specification at that point, and the C standard says using an invalid conversion specification is
undefined behaviour - in practice this usually just misreads the argument list, but nothing prevents a
library from doing something worse with it.

## How to fix

Before:
```cpp
#include <cstdio>
void f() {
    printf("%I", 5); // <- 'I' with no conversion specifier after it
}
```

After:
```cpp
#include <cstdio>
void f() {
    printf("%Id", (ptrdiff_t)5);
}
```
