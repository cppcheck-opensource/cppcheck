# checkCastIntToCharAndBack

**Message**: Storing getchar() return value in char variable and then comparing with EOF.<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

The return value of `getchar()`/`getc()`/`fgetc()` (an `int`, so it can represent every possible byte
value plus the special value `EOF`) is stored in a `char` and *then* compared with `EOF` - once narrowed
to `char`, a real byte that happens to equal the platform's `EOF` value as a `char` can be mistaken for
end-of-file, or vice versa.

## Motivation

`getchar()` and friends return `int` specifically so that every possible `unsigned char` byte value
(0-255) and the distinct sentinel value `EOF` can all be told apart. Storing the result in a `char`
first throws away exactly the information needed to make that distinction - depending on the platform's
`char` signedness and `EOF`'s value, a legitimate byte can end up equal to `EOF` after narrowing, causing
input to be truncated early, or (less commonly) `EOF` itself to go unrecognized.

## How to fix

Keep the return value in an `int` until after it has been compared with `EOF`.

Before:
```cpp
#include <cstdio>
void bar(char);
void f() {
    unsigned char c;
    c = getchar();
    while (c != EOF) { // <- 'c' can never actually equal EOF once narrowed
        bar(c);
        c = getchar();
    }
}
```

After:
```cpp
#include <cstdio>
void bar(int);
void f() {
    int c;
    c = getchar();
    while (c != EOF) {
        bar(c);
        c = getchar();
    }
}
```

## Related checkers

- [charBitOp.md](charBitOp.md) - a different `char`-signedness pitfall, about sign extension in bitwise
  operations rather than narrowing a stream-read result.
