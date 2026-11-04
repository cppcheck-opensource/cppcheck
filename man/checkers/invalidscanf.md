# invalidscanf

**Message**: scanf() without field width limits can crash with huge input data.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

A `scanf`-family call reads a string (`%s`, or a character set `%[...]`) with no maximum field width -
given long enough input, this overflows the destination buffer.

## Motivation

Without a field width, `scanf("%s", buf)` (and its relatives) will write as much input as it's given,
with no regard for the size of `buf` - a classic, easily-exploitable buffer overflow driven entirely by
the input data.

## How to fix

Before:
```cpp
#include <cstdio>
void f() {
    char c[5];
    scanf("%s", c); // <- no width limit, can overflow 'c'
}
```

After:
```cpp
#include <cstdio>
void f() {
    char c[5];
    scanf("%4s", c);
}
```
