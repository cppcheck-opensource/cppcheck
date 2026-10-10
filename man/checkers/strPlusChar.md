# strPlusChar

**Message**: Unusual pointer arithmetic. A value of type 'char' is added to a string literal.<br/>
**Category**: Correctness<br/>
**Severity**: Error<br/>
**Language**: C/C++

## Description

A string literal has a `char`/`wchar_t` value added to it with `+`, with the literal written on the
left-hand side (`"text" + ch`) - the same expression with the operands swapped (`ch + "text"`) is not
covered by this check.

## Motivation

`"/usr" + '/'` doesn't concatenate the character onto the string - a string literal decays to a
`const char*`, so `+` here is pointer arithmetic: it adds the character's numeric value to the pointer,
producing a pointer into the middle of (or past the end of) the literal. This looks like string
concatenation but is nothing like it.

This can lead to undefined behaviour by itself, separate from whatever happens if the pointer is later
used: the C++ standard only allows pointer arithmetic to land inside the array or exactly one past its
end, and a character value large enough to land outside that range (as `'/'` does against the 5-byte
`"/usr"`) makes the addition itself undefined behaviour, whether or not the resulting pointer is ever
dereferenced. cppcheck doesn't check whether the specific character value would actually stay in range -
this is flagged purely because the syntactic pattern (a string literal plus a `char`) is essentially
always a concatenation mistake, not because cppcheck has determined that this particular addition goes
out of bounds.

## How to fix

Before:
```cpp
void foo() {
    const char *p = "/usr" + '/'; // <- pointer arithmetic on a string literal
}
```

After:
```cpp
#include <string>
void foo() {
    std::string p = std::string("/usr") + '/';
}
```
