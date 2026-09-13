# va_start_referencePassed

**Message**: Using reference 'x' as parameter for va_start() results in undefined behaviour.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C/C++

## Description

A reference parameter is passed as the second argument to `va_start()`. This is undefined behaviour.

## Motivation

`va_start()` needs the actual last named parameter as it's stored on the stack, not a reference to it -
a reference is a different object from what the calling convention expects at that point, so using one
here doesn't reliably locate the variadic arguments that follow.

## How to fix

Avoid declaring the last named parameter before `...` as a reference, or otherwise ensure `va_start()`
is given the real parameter rather than a reference to it.

## Related checkers

- [va_start_wrongParameter.md](va_start_wrongParameter.md), [va_list_usedBeforeStarted.md](va_list_usedBeforeStarted.md), [va_start_subsequentCalls.md](va_start_subsequentCalls.md), [va_end_missing.md](va_end_missing.md) - other misuses of the same `va_list`/`va_start()`/`va_end()` facility.
