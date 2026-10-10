# extraWhitespaceError

**Message**: Found extra whitespace in the pattern.<br/>
**Category**: Code Quality<br/>
**Severity**: Warning<br/>
**Language**: C++ (cppcheck's own source code only)

## Description

A pattern string has leading/trailing or doubled whitespace, which doesn't change what it matches but
suggests a typo.

This is not a general-purpose checker for arbitrary C/C++ programs - see
[simplePatternError.md](simplePatternError.md) for the shared background on this internal,
cppcheck-source-only checker.

## Motivation

Stray whitespace in a pattern string is harmless to how the pattern matches, but it's a signal that the
string may have been edited carelessly (for example, a token accidentally deleted without removing its
surrounding space) - worth a second look even though it isn't a functional bug on its own.

## How to fix

Remove the extra leading, trailing, or doubled whitespace from the pattern string.
