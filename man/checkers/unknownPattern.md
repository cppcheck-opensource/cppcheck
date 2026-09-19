# unknownPattern

**Message**: Unknown pattern used: "%typex%"<br/>
**Category**: Code Quality<br/>
**Severity**: Error<br/>
**Language**: C++ (cppcheck's own source code only)

## Description

A `%something%` placeholder isn't one of the pattern language's recognized names.

This is not a general-purpose checker for arbitrary C/C++ programs - see
[simplePatternError.md](simplePatternError.md) for the shared background on this internal,
cppcheck-source-only checker.

## Motivation

An unrecognized placeholder name is almost always a typo for a real one (`%type%` misspelled as
`%typex%`, for example) - since the pattern engine doesn't know what it means, the match silently fails
to do what was intended.

## How to fix

Correct the placeholder name to one the pattern language actually recognizes (`%type%`, `%var%`,
`%num%`, ...).

## Related checkers

- [missingPercentCharacter.md](missingPercentCharacter.md) - a placeholder that's missing its closing `%` entirely, rather than misspelled.
