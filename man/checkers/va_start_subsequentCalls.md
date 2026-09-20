# va_start_subsequentCalls

**Message**: va_start() or va_copy() called subsequently on 'args' without va_end() in between.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C/C++

## Description

`va_start()` or `va_copy()` is called again on a `va_list` that is already open, without an intervening
`va_end()`.

## Motivation

Reopening a `va_list` that's still open, without closing it first, is undefined behaviour on many
implementations - the two calls to `va_start()`/`va_copy()` can leave the `va_list` in an inconsistent
state, and whatever `va_arg()` reads afterward is unreliable.

## How to fix

Call `va_end()` on a `va_list` before opening it again with `va_start()`/`va_copy()`.

## Related checkers

- [va_start_wrongParameter.md](va_start_wrongParameter.md), [va_start_referencePassed.md](va_start_referencePassed.md), [va_list_usedBeforeStarted.md](va_list_usedBeforeStarted.md), [va_end_missing.md](va_end_missing.md) - other misuses of the same `va_list`/`va_start()`/`va_end()` facility.
