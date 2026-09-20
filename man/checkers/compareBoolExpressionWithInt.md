# compareBoolExpressionWithInt

**Message**: Comparison of a boolean expression with an integer other than 0 or 1.<br/>
**Category**: Code Quality<br/>
**Severity**: Warning<br/>
**Language**: C++

## Description

A boolean expression (not necessarily a `bool` variable - for example `a && b`, or a comparison) is
compared against a number.

## Motivation

A boolean expression only ever evaluates to `0` or `1`, so comparing it against any other number is
always false, and comparing it against `0`/`1` with a relational operator is easy to get backwards -
either way, this is rarely what the code's author actually intended.

## How to fix

Compare the boolean expression directly, or with `==`/`!=` against `true`/`false`, instead of against
an arbitrary integer.
