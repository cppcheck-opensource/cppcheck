# redundantNextPrevious

**Message**: Statement is redundant, code can be simplified.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C++ (cppcheck's own source code only)

## Description

A chain like `tok->next()->previous()` can be simplified (here, to just `tok`).

This is not a general-purpose checker for arbitrary C/C++ programs - see
[simplePatternError.md](simplePatternError.md) for the shared background on this internal,
cppcheck-source-only checker.

## Motivation

Walking forward and then immediately back (or vice versa) always returns to the starting token - the
extra calls add nothing but noise and a small amount of wasted work.

## How to fix

Before:
```cpp
return tok->next()->previous(); // <- redundant
```

After:
```cpp
return tok;
```

## Related checkers

- [redundantTokCheck.md](redundantTokCheck.md) - a different kind of redundant code involving the same `Token`/pattern-matching API.
