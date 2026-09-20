# commaSeparatedReturn

**Message**: Comma is used in return statement. The comma can easily be misread as a ';'.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

A comma appears inside a `return` statement (`return a + 1, b++;`) - this is a single statement using
the comma operator, easily misread as `return a + 1;` followed by a separate `b++;`.

**This check is currently switched off in cppcheck and never produces a warning**, even for the exact
code pattern it's meant to catch - it's kept in the error list only so its ID stays recognized (for
example in older suppression files), not because it's active. Don't rely on it to catch this mistake.

## Motivation

The comma operator lets one statement evaluate several expressions in sequence, discarding all but the
last one's value - `return a + 1, b++;` actually returns the value of `b++`, not `a + 1`. This reads
exactly like two separate statements with a typo'd semicolon, which is a very easy way to misjudge what
value a function returns.

## How to fix

Split the comma expression into separate statements, and make the intended return value explicit.

Before:
```cpp
int f(int x, int a, int b) {
    if (x)
        return a + 1,   // looks like this is returned...
    b++;                // ...but this is actually returned instead
    return 0;
}
```

After:
```cpp
int f(int x, int a, int b) {
    if (x) {
        b++;
        return b;
    }
    return 0;
}
```

