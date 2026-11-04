# coutCerrMisusage

**Message**: Invalid usage of output stream: '<< std::cout'.<br/>
**Category**: Correctness<br/>
**Severity**: Error<br/>
**Language**: C++

## Description

`std::cout`/`std::cerr` is streamed into itself (`std::cout << std::cout;`) - almost always a typo for
a variable that was meant to be printed.

## Motivation

Streaming `std::cout` into itself doesn't print anything meaningful and is essentially always a typo -
usually a variable name was meant to be there instead of the stream's own name.

## How to fix

Before:
```cpp
#include <iostream>
void f() {
    std::cout << std::cout; // <- likely a typo'd variable name
}
```

After:
```cpp
#include <iostream>
void f() {
    std::cout << "hello";
}
```
