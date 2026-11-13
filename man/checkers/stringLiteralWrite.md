# stringLiteralWrite

**Message**: Modifying string literal "abc" directly or indirectly is undefined behaviour.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C/C++

## Description

A string literal is modified through a pointer to it (directly, or via a pointer passed to another
function).

## Motivation

String literals are typically stored in read-only memory, so writing to one crashes or corrupts memory
unpredictably depending on the platform. `char *p = "abc";` compiles without warning even though `p`
points at memory that must not be written to.

## How to fix

Before:
```cpp
void f() {
    char *abc = "abc";
    abc[0] = 'a'; // <- undefined behaviour
}
```

After:
```cpp
void f() {
    char abc[] = "abc"; // a real, modifiable array
    abc[0] = 'a';
}
```
