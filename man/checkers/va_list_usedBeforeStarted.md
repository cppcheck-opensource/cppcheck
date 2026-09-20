# va_list_usedBeforeStarted

**Message**: va_list 'args' used before va_start() was called.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C/C++

## Description

A `va_list` variable is read - passed to `va_arg()`, `va_copy()` or `va_end()` - before `va_start()`
(or `va_copy()`) was called on it, or after it was already closed with `va_end()`.

## Motivation

A `va_list` only refers to a valid argument sequence between a matching `va_start()`/`va_copy()` and the
`va_end()` that closes it. Reading it outside that window - too early, or again after it's already been
closed - is undefined behaviour, since there's no argument sequence for it to actually point to.

## How to fix

Only read a `va_list` after it has been opened with `va_start()`/`va_copy()`, and before it is closed
with `va_end()`.

Before:
```cpp
void log(const char* fmt, ...) {
    va_list args;
    int first = va_arg(args, int); // <- used before va_start()
    va_start(args, fmt);
    va_end(args);
}
```

After:
```cpp
void log(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int first = va_arg(args, int);
    va_end(args);
}
```

## Related checkers

- [va_start_wrongParameter.md](va_start_wrongParameter.md), [va_start_referencePassed.md](va_start_referencePassed.md), [va_start_subsequentCalls.md](va_start_subsequentCalls.md), [va_end_missing.md](va_end_missing.md) - other misuses of the same `va_list`/`va_start()`/`va_end()` facility.
