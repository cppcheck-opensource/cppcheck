# incorrectStringBooleanError and incorrectCharBooleanError

**Message**: Conversion of string literal "Hello" to bool always evaluates to true.<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

A string or char literal is used directly where a boolean is expected (`if ("Hello")`,
`x ? "a" : "b"` as the whole condition) - this is always `true` unless the literal is exactly
`""`/`'\0'`.

## Motivation

A non-empty string or character literal converts to `true` every time - if the intent was to check a
variable's value, using the literal itself instead is a typo that silently makes the condition
constant, so the code inside always (or never) runs regardless of any real input.

## How to fix

Before:
```cpp
int f() {
    if ("Hello") {} // <- always true
    return 0;
}
```

After:
```cpp
int f(bool flag) {
    if (flag) {}
    return 0;
}
```
