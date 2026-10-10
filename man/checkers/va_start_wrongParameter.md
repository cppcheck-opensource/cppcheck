# va_start_wrongParameter

**Message**: 'level' given to va_start() is not last named argument of the function. Did you intend to pass 'fmt'?<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

The parameter passed as the second argument to `va_start()` is not the function's actual last named
parameter (the one right before `...`).

## Motivation

`va_start()` needs the last named parameter to locate where the variadic arguments begin on the stack.
Passing any other parameter is undefined behaviour - `va_arg()` calls that follow can then read garbage
instead of the actual variadic arguments, and the exact symptom depends on the compiler and calling
convention.

## How to fix

Pass the function's true last named parameter to `va_start()`.

Before:
```cpp
void log(const char* fmt, const char* tag, int level, ...) {
    va_list args;
    va_start(args, fmt); // <- 'level' is the actual last named parameter
    va_end(args);
}
```

After:
```cpp
void log(const char* fmt, const char* tag, int level, ...) {
    va_list args;
    va_start(args, level);
    va_end(args);
}
```

## Related checkers

- [va_start_referencePassed.md](va_start_referencePassed.md), [va_list_usedBeforeStarted.md](va_list_usedBeforeStarted.md), [va_start_subsequentCalls.md](va_start_subsequentCalls.md), [va_end_missing.md](va_end_missing.md) - other misuses of the same `va_list`/`va_start()`/`va_end()` facility.
