# orInComplexPattern

**Message**: Found \"|\" in complex pattern. You probably intend to use \"%or%\".<br/>
**Category**: Code Quality<br/>
**Severity**: Error<br/>
**Language**: C++ (cppcheck's own source code only)

## Description

A pattern uses a literal `|`/`||` instead of the pattern language's `%or%`/`%oror%`.

This is not a general-purpose checker for arbitrary C/C++ programs - see
[simplePatternError.md](simplePatternError.md) for the shared background on this internal,
cppcheck-source-only checker.

## Motivation

A literal `|`/`||` character in a pattern string matches the literal token `|`/`||` in the code being
analyzed - it does not mean "either of these two alternatives" the way `%or%`/`%oror%` does. This is an
easy habit to slip into for anyone used to regular-expression syntax, and it silently changes what the
pattern matches.

## How to fix

Replace the literal `|`/`||` with `%or%`/`%oror%` when the intent is "match either of these token
types", keeping the literal form only when the code being analyzed should actually contain a `|`/`||`
character.

## Related checkers

- [unknownPattern.md](unknownPattern.md) - a different kind of placeholder mistake in the same pattern language.
