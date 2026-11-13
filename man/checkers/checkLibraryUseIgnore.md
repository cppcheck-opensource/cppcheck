# checkLibraryUseIgnore

**Message**: --check-library: Function f() should have <use>/<leak-ignore> configuration<br/>
**Category**: Configuration<br/>
**Severity**: Information<br/>
**Language**: C/C++

## Description

Only produced with `--check-library`: a function call couldn't be classified as either using or
ignoring a tracked (allocated) variable passed into it. This is aimed at people writing library
configurations, not at application code.

## Motivation

cppcheck's leak-tracking checks ([memleak.md](memleak.md) and its siblings) rely on knowing, for every
function a tracked variable is passed to, whether that function takes ownership of it, merely reads it,
or does something else. When a library configuration doesn't say, `--check-library` flags the gap so
the configuration can be completed rather than silently guessing.

## How to fix

Add a `<use>` or `<leak-ignore>` entry for the function in the relevant library configuration file, so
cppcheck knows how it treats the argument.

## Related checkers

- [memleak.md](memleak.md) and its siblings - the actual leak/double-free/use-after-free checks that
  this configuration gap affects the accuracy of.
