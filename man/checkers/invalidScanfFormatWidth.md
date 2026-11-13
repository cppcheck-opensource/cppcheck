# invalidScanfFormatWidth and invalidScanfFormatWidth_smaller

**Message**: Width 9 given in format string (no. 1) is larger than destination buffer 'str[8]', use %8c to prevent overflowing it.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error/Warning<br/>
**Language**: C/C++

## Description

- `invalidScanfFormatWidth`: a `%<width>s`/`%<width>c` field width is *larger* than the destination
  array - `scanf` doesn't stop at the array's bound, only at the field width, so this overflows the
  buffer.
- `invalidScanfFormatWidth_smaller`: the field width is *smaller* than the destination array - not a
  bug, but a hint that the width might have been computed wrong (this variant needs `--inconclusive` to
  show, since it's just a suspicious-looking number, not a proven mistake).

## Motivation

`scanf` writes up to as many characters as the field width says, regardless of how big the destination
buffer actually is - if the width is larger than the buffer, the read overflows it as soon as the input
actually supplies that many characters (given short enough input, it may not overflow on a particular
run, which is what makes this easy to miss in testing); if the width is smaller than the buffer, it
just leaves the buffer holding less than expected. Neither mistake is visible from the call site alone.

## How to fix

Before:
```cpp
#include <cstdio>
void f() {
    char str[8];
    scanf("%9c", str); // <- invalidScanfFormatWidth: 9 > 8, overflows 'str'
}
```

After:
```cpp
#include <cstdio>
void f() {
    char str[8];
    scanf("%8c", str);
}
```

Before:
```cpp
#include <cstdio>
void f() {
    char str[10];
    scanf("%5s", str); // <- invalidScanfFormatWidth_smaller: 5 is well under 10, worth double-checking
}
```

After:
```cpp
#include <cstdio>
void f() {
    char str[10];
    scanf("%9s", str);
}
```
