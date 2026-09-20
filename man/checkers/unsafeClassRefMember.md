# unsafeClassRefMember

**Message**: Storing reference to member argument in the class member is unsafe.<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C++

## Description

A `const&` member is initialized directly from a `const&` constructor argument - if a caller ever
passes a temporary or a short-lived local variable there, the member ends up referring to something
that's already been destroyed.

This check only runs when analysis is configured through a Cppcheck GUI/project file that turns on
"safe checks" for classes (`<safe-checks><class-public/></safe-checks>`) - there is no plain
command-line flag for it, so it will not appear in an ordinary `--enable=...` run.

## Motivation

Storing a reference member straight from a constructor's reference parameter looks harmless, but it
makes the object's validity depend entirely on the lifetime of whatever the caller happened to pass in -
if that argument was a temporary, the member becomes a dangling reference the moment the constructor
call's statement finishes, and every later use of the member is undefined behaviour. This check fires on
the constructor's signature alone - it doesn't (and can't, from the class definition by itself) know what
every caller actually passes in, so a class that's only ever constructed with a long-lived object is
flagged just the same as one that's handed a temporary.

## How to fix

Store a copy of the value instead of a reference to it, unless the class's contract explicitly requires
the referred-to object to outlive it.
