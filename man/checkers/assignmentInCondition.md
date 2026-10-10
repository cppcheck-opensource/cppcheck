# assignmentInCondition

**Message**: Suspicious assignment in condition. Condition 't=s' is always true.<br/>
**Category**: Correctness<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

A container or iterator is assigned (`=`) directly inside a condition, where `==` was probably meant -
as written, the condition is always true.

## Motivation

`if (t = s)` assigns `s` to `t` and then tests the (always-true, for a container/iterator) result of
that assignment - almost certainly a typo for `if (t == s)`. Because it's valid, compiling code, this is
easy to miss in review.

## How to fix

Before:
```cpp
#include <string>
void f(const std::string& s) {
    std::string t;
    if (t = s) {} // <- always true, did you mean '=='?
}
```

After:
```cpp
#include <string>
void f(const std::string& s) {
    std::string t = s;
    if (!t.empty()) {}
}
```

## Related checkers

- [clarifyCondition.md](clarifyCondition.md) - a related but distinct mistake: an assignment or bitwise
  operator next to a comparison with ambiguous precedence.
