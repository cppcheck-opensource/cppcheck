# knownConditionTrueFalse

**Message**: Condition 'x==5' is always true<br/>
**Category**: Correctness<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

cppcheck can already work out this condition's value from what it knows about the variables involved,
so the condition is always true or always false.

## Motivation

A condition that's always true or always false isn't testing anything - at best it's confusing,
leftover, or dead code; at worst, it means the intended check never actually happens and a real bug (a
wrong comparison, a typo'd variable, a value that was supposed to vary but doesn't) slips through
unnoticed.

This check may need `--check-level=exhaustive` to see every case.

## How to fix

Before:
```cpp
void f() {
    int x = 5;
    if (x == 5) {} // <- always true
}
```

After: use the real variable instead of a fixed value, or remove the redundant check.
```cpp
void f(int x) {
    if (x == 5) {}
}
```

## False positives to be aware of

- **This check does not account for a member value changing through a call that reaches it indirectly**
  (for example, through a container of pointers the function iterates over). A member read before such
  a call can be wrongly assumed to still hold the same value afterwards:
  ```cpp
  #include <map>
  #include <string>
  struct S { int i; };
  struct T {
      std::map<std::string, S*> m;
      S* get(const std::string& s) { return m[s]; }
      void modify() { for (const auto& e : m) e.second->i = 0; }
  };
  void f(T& t) {
      const S* p = t.get("abc");
      const int o = p->i;
      t.modify();          // this can change p->i
      if (p->i == o) {}    // wrongly reported as always true
  }
  ```

## Related checkers

- [assignIfError.md](assignIfError.md) - the same idea, specifically for a value just narrowed down by
  a bitmask assignment.
- [moduloAlwaysTrueFalse.md](moduloAlwaysTrueFalse.md), [compareValueOutOfTypeRangeError.md](compareValueOutOfTypeRangeError.md) -
  the same idea, for a `%` result or a type's value range instead of a general known value.
- [duplicateConditionalAssign.md](duplicateConditionalAssign.md) - a related, narrower case: an
  assignment that repeats a value a condition already guarantees.
