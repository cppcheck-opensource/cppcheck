# varFuncNullUB

**Message**: Passing NULL after the last typed argument to a variadic function leads to undefined behaviour.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Portability<br/>
**Language**: C/C++

## Description

`NULL` is passed as the last argument to a variadic function (`...`) - on platforms where `NULL` is
defined as a plain `0` rather than a pointer-sized constant, the function reading its arguments through
`va_arg()` may misinterpret it, since the two constants aren't guaranteed to be the same size.

## Motivation

`NULL`'s definition (`0`, `0L`, or `(void*)0`) is implementation-defined. On a platform where
`sizeof(int) != sizeof(void*)` and `NULL` expands to a plain integer `0`, a variadic function expecting
to read a pointer-sized sentinel through `va_arg()` reads the wrong number of bytes - which can crash or
silently read garbage for the following arguments. The bug is invisible on platforms where `NULL`
happens to be pointer-sized, so it can go unnoticed for a long time.

## How to fix

Cast the sentinel to the pointer type the function actually expects, instead of relying on `NULL`'s
platform-specific definition.

Before:
```cpp
void a(...);
void b() { a(NULL); } // <- passing NULL as the last variadic argument
```

After:
```cpp
void a(...);
void b() { a((void*)0); }
```
