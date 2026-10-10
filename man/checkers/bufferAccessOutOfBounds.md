# bufferAccessOutOfBounds

**Message**: Buffer is accessed out of bounds: d<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error/Warning<br/>
**Language**: C/C++

## Description

A call to a function cppcheck has size information for (`strcpy`, `strcat`, `memcpy`, `sprintf`, and
similar, via the library configuration) is passed a buffer that's too small for what the call can read
or write.

## Motivation

Many standard library functions read or write a caller-supplied buffer without any bounds checking of
their own - it's entirely up to the caller to make sure the buffer is large enough. Getting this wrong
is a very common, very old source of buffer overruns in C/C++ code.

## How to fix

Before:
```cpp
void f() {
    char d[3] = {};
    strcat(d, "12345678"); // <- 'd' can't hold this much
}
```

After:
```cpp
void f() {
    char d[10] = {};
    strcat(d, "12345678");
}
```

