# raceAfterInterlockedDecrement

**Message**: Race condition: non-interlocked access after InterlockedDecrement(). Use InterlockedDecrement() return value instead.<br/>
**Category**: Correctness<br/>
**Severity**: Error<br/>
**Language**: Windows platform only

## Description

Code checks a variable's value directly right after calling the Windows `InterlockedDecrement()` API on
it - between the API call and the check, another thread could have already changed the variable again,
so only the value `InterlockedDecrement()` itself returned can be trusted.

## Motivation

`InterlockedDecrement()` exists specifically to make the decrement-and-check atomic across threads. If
the code decrements the variable and then reads it again as a separate step, another thread can run in
between and change the value - the whole point of using the interlocked API is defeated, reintroducing
the exact race condition it was meant to prevent. cppcheck recognizes this purely from the code shape (an
`InterlockedDecrement()` call immediately followed by a plain re-read of the same variable) - it has no
way to confirm another thread genuinely touches that variable, so this is a strong hint of a real race,
not a proof that one exists in every case it's reported.

## How to fix

Use the value `InterlockedDecrement()` itself returns, instead of re-reading the variable afterwards.

Before:
```cpp
void destroy();
void f() {
    int counter = 0;
    InterlockedDecrement(&counter);
    if (counter) // <- another thread could change 'counter' in between
        return;
    destroy();
}
```

After:
```cpp
void destroy();
void f() {
    int counter = 0;
    if (InterlockedDecrement(&counter) == 0)
        return;
    destroy();
}
```
