# invalidFunctionArgBool

**Message**: Invalid $symbol() argument nr 1. A non-boolean value is required.<br/>
**Category**: Correctness<br/>
**Severity**: Error<br/>
**Language**: C/C++

## Description

A plain `0`/`1` is passed where a function (per its library configuration) actually requires a real
boolean argument.

## Motivation

Some functions distinguish a genuine boolean argument from an arbitrary integer that happens to be `0`
or `1` - passing the wrong kind of value can be accepted silently but not do what's intended.

## How to fix

Before:
```cpp
void setFlag(bool enabled);
void f() {
    setFlag(1); // <- treat this as a real boolean, not an arbitrary int
}
```

After:
```cpp
void setFlag(bool enabled);
void f() {
    setFlag(true);
}
```

## Related checkers

- [invalidFunctionArg.md](invalidFunctionArg.md) - the same idea, but for an argument whose numeric
  value must fall in a specific range.
- [invalidFunctionArgStr.md](invalidFunctionArgStr.md) - the same idea, but for an argument that needs
  a null-terminated C-string.
