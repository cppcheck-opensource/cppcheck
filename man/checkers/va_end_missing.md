# va_end_missing

**Message**: va_list 'args' was opened but not closed by va_end().<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C/C++

## Description

A local `va_list` variable is opened with `va_start()`/`va_copy()`, but the function can finish without
it being closed by `va_end()`.

## Motivation

Every `va_list` opened with `va_start()`/`va_copy()` must be matched with exactly one `va_end()` call
before it goes out of scope. Leaving one open is undefined behaviour on some implementations, and can
also leak resources the implementation associated with iterating the variadic arguments.

## How to fix

Call `va_end()` on every path that leaves the function after `va_start()`.

Before:
```cpp
void log(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
} // <- va_end() never called
```

After:
```cpp
void log(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    va_end(args);
}
```

## Related checkers

- [va_start_wrongParameter.md](va_start_wrongParameter.md), [va_start_referencePassed.md](va_start_referencePassed.md), [va_list_usedBeforeStarted.md](va_list_usedBeforeStarted.md), [va_start_subsequentCalls.md](va_start_subsequentCalls.md) - other misuses of the same `va_list`/`va_start()`/`va_end()` facility.
