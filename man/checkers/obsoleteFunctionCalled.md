# &lt;name&gt;Called (for example getsCalled, bsd_signalCalled, or any other function marked this way in a library configuration)

**Message**: Obsolete function 'gets' called. It is recommended to use 'fgets' or 'gets_s' instead.<br/>
**Category**: Correctness<br/>
**Severity**: Error/Warning/Style/Portability<br/>
**Language**: C/C++

## Description

A function known to be obsolete, dangerous, or non-reentrant is called. The message names a safer
replacement. The exact error ID is built from the function's own name plus `Called` (`getsCalled`,
`bsd_signalCalled`, ...), and which functions are covered - and at what severity - comes entirely from
the loaded library configuration (`std.cfg`, `posix.cfg`, and similar), not from a fixed list built into
cppcheck itself. `alloca()` is handled separately - see [allocaCalled.md](allocaCalled.md).

## Motivation

Some standard library functions are obsolete or unsafe for reasons that aren't visible from their
signature alone: `gets()` can't bound how much it reads and will happily overflow any buffer, some
functions from `<signal.h>` are unreliable across platforms, and so on. A caller has no way to know this
without already being aware of the function's history.

## How to fix

Before:
```cpp
#include <cstdio>
void f(char *a) {
    char *x = gets(a); // <- gets() cannot bound the input, buffer overflow risk
}
```

After:
```cpp
#include <cstdio>
void f(char* buf, int n) {
    char *x = fgets(buf, n, stdin);
}
```

