# simplePatternError

**Message**: Found simple pattern inside Token::Match() call: ";"<br/>
**Category**: Code Quality<br/>
**Severity**: Warning<br/>
**Language**: C++ (cppcheck's own source code only)

## Description

A pattern given to `Token::Match()`/`Token::findmatch()` doesn't actually use any wildcard syntax, so
the cheaper `Token::simpleMatch()`/`Token::findsimplematch()` could be used instead.

This is not a general-purpose checker for arbitrary C/C++ programs. It only looks for calls to
cppcheck's own internal pattern-matching API, and reports style/correctness issues in how that API is
used. It exists to help cppcheck's own contributors write correct and efficient code in the cppcheck
codebase, not to check any project being analyzed by cppcheck.

It is only compiled in when cppcheck itself is built with the `CHECK_INTERNAL` macro defined - a normal
end-user build of cppcheck does not have it at all, and even a build that has it still needs
`--enable=internal` to turn it on.

## Motivation

`Token::Match()` interprets a small wildcard syntax in its pattern string, which costs more than a
plain text comparison. When a pattern doesn't use any of that syntax, `Token::simpleMatch()` does the
identical comparison faster.

## How to fix

Before:
```cpp
Token::Match(tok, ";"); // <- no wildcard syntax used
```

After:
```cpp
Token::simpleMatch(tok, ";");
```

## Related checkers

- [complexPatternError.md](complexPatternError.md) - the opposite mistake, using `simpleMatch()` with a pattern that needs wildcard interpretation.
