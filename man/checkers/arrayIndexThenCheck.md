# arrayIndexThenCheck

**Message**: Array index 'i' is used before limits check.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

An array is indexed with a variable, and only *afterwards* is that same variable checked against its
limits (`a[i] && i < 10`) in a way that looks like it was meant to guard the access. Even where it's
not currently a bug, this ordering means a bounds check that no longer matches the access (after later
edits) won't protect anything.

## Motivation

Writing the bounds check after the access it's meant to guard defeats the purpose of a short-circuiting
`&&`/`||`: by the time the check runs, the access has already happened. Even in cases where the access
happens to be safe today, this ordering is fragile - it looks protective without actually being so, and
a later edit to the bounds is easy to get wrong without anything catching it.

## How to fix

Before:
```cpp
void f(const char s[], int i) {
    if (s[i] == 'x' && i < 20) { // <- index used before the limit check
    }
}
```

After:
```cpp
void f(const char s[], int i) {
    if (i < 20 && s[i] == 'x') {
    }
}
```
