# complexPatternError

**Message**: Found complex pattern inside Token::simpleMatch() call: "%type%"<br/>
**Category**: Code Quality<br/>
**Severity**: Error<br/>
**Language**: C++ (cppcheck's own source code only)

## Description

A pattern given to `Token::simpleMatch()`/`Token::findsimplematch()` contains wildcard syntax
(`%type%`, `[abc]`, `a|b`, ...) that those "simple" functions don't actually interpret; they only do a
literal, word-by-word text comparison.

This is not a general-purpose checker for arbitrary C/C++ programs - see
[simplePatternError.md](simplePatternError.md) for the shared background on this internal,
cppcheck-source-only checker.

## Motivation

Mixing up `Match()` with `simpleMatch()` silently loses the wildcard matching a pattern was written to
rely on - the code compiles and runs, but the comparison never matches the way the author intended,
turning into a subtle logic bug in cppcheck's own analysis.

## How to fix

Before:
```cpp
Token::simpleMatch(tok, "%type%"); // <- simpleMatch() won't interpret this
```

After:
```cpp
Token::Match(tok, "%type%");
```

## Related checkers

- [simplePatternError.md](simplePatternError.md) - the opposite mistake, using `Match()` with a pattern that has no wildcard syntax at all.
