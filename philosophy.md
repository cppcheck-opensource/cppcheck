
# Cppcheck Philosophy

It is important that everybody in the Cppcheck team has a consistent idea about how this tool should work.

This is a static analyzer tool.

## Usability

Usability is very important. It's more important that Cppcheck is usable than finding all bugs.
 - We don't want to have tons of configurations options.
 - It's very important that warning messages are well written and with enough details.
 - Speed is very important. --check-level=exhaustive can be used when user accept slow analysis.

## Normal analysis - No false positives

A fundamental goal is "no false positives".

It is not possible to achieve "no false positives" completely. One case where false positives are OK is when the code is garbage.

If the code is written as it is by design, then our goal is to not show any false positives.

If it is not known if there is a problem, then in general we need to bailout to avoid false positives. We can only warn when we see that there is a problem.

Stylistic checks are much more prone to false positives and therefore we should avoid writing stylistic checks mostly.

Reporting issues in Trac:
 - If you see a false negative; report that as an enhancement.
 - If you see a false positive; report that as a defect.

### Inconclusive messages

If cppcheck can't determine that there is a problem or not, then the analysis is inconclusive.

If the user enables inconclusive warnings and we guess that the probability there is a real problem is at least 50-50 then it's OK to write a inconclusive warning.

Inconclusive messages shall not be used for new checks which are just being developed. There `settings.experimental` can be used.


## No configuration

We want that a user can run Cppcheck without explicit -D and -I configuration.

When this happens the false positives should be avoided. The user can reduce false negatives with configuration.


## Allow compiler extensions

This is not just a tool for mainstream gcc/msvc c/c++ developers. If you can compile the code with a C/C++ compiler then our goal is that Cppcheck can check it.


## C++ language

Our goal is to be highly portable. Users must be able to compile Cppcheck with GCC 4.8 or Visual Studio 2013.

No C++14 is allowed. A subset of C++11 is allowed.


## Avoid dependencies

We are very careful about dependencies.



