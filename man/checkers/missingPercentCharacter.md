# missingPercentCharacter

**Message**: Missing percent character in Token::Match() pattern: "%type"<br/>
**Category**: Code Quality<br/>
**Severity**: Error<br/>
**Language**: C++ (cppcheck's own source code only)

## Description

A `%something` placeholder is missing its closing `%`.

This is not a general-purpose checker for arbitrary C/C++ programs - see
[simplePatternError.md](simplePatternError.md) for the shared background on this internal,
cppcheck-source-only checker.

## Motivation

Without the closing `%`, the pattern-matching engine doesn't recognize the text as a placeholder at
all, and instead tries to match it as literal characters - silently breaking the match the pattern was
written to perform.

## How to fix

Before:
```cpp
Token::Match(tok, "%type"); // <- missing closing '%'
```

After:
```cpp
Token::Match(tok, "%type%");
```

## Related checkers

- [unknownPattern.md](unknownPattern.md) - a `%something%` placeholder that's correctly closed but isn't a recognized name.
