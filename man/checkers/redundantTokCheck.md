# redundantTokCheck

**Message**: Unnecessary check of token; 'Token::Match()' already checks if it is a nullptr.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C++ (cppcheck's own source code only)

## Description

Code like `if (tok && Token::Match(tok, "..."))` - the `tok &&` part is redundant, since these
`Token::*` functions already return false/no-match when given a null token.

This is not a general-purpose checker for arbitrary C/C++ programs - see
[simplePatternError.md](simplePatternError.md) for the shared background on this internal,
cppcheck-source-only checker.

## Motivation

Since `Token::Match()` and its relatives already handle a null token safely by returning "no match", an
explicit null check right before calling one of them adds nothing but visual clutter.

## How to fix

Before:
```cpp
if (tok && Token::Match(tok, "foo")) {} // <- 'tok &&' is unnecessary
```

After:
```cpp
if (Token::Match(tok, "foo")) {}
```

## Related checkers

- [redundantNextPrevious.md](redundantNextPrevious.md) - a different kind of redundant code involving the same `Token` API.
