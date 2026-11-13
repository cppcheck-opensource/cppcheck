# autovarInvalidDeallocation

**Message**: Deallocating a pointer that was not dynamically allocated: tmp<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C/C++

## Description

`free()`/`delete` is called on something that wasn't allocated dynamically - a local array, a string
literal, or the address of a local/global/static variable.

## Motivation

`free()`/`delete` are only valid on memory that was actually obtained from the matching allocation
function. Calling either on something else - stack memory, a string literal, an unrelated variable's
address - is undefined behaviour, typically corrupting the memory allocator's own bookkeeping.

## How to fix

Before:
```cpp
void foo() {
    char tmp[256];
    free(tmp); // <- autovarInvalidDeallocation: 'tmp' isn't heap-allocated
}
```

After:
```cpp
void foo() {
    char *tmp = malloc(256);
    free(tmp);
}
```
