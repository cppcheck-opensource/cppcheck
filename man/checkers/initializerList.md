# initializerList

**Message**: Member variable 'c' is in the wrong order in the initialization list.<br/>
**Category**: Correctness<br/>
**Severity**: Style<br/>
**Language**: C++

## Description

A constructor's member-initializer list either lists members in a different order than they're declared
in the class (misleading, since members are always initialized in declaration order, not
initializer-list order), or initializes one member using another member as a source value when that
source member is declared *after* it - so, despite appearing earlier in the list, the source hasn't
actually been initialized yet at that point.

## Motivation

C++ always initializes members in the order they're *declared* in the class, regardless of what order
they're written in the initializer list. cppcheck flags any mismatch between the two orders purely as a
defensive-programming measure - most of the time (as in the first example below, where every member is
just initialized from a constant) writing the list out of order is only misleading to a reader, with no
real consequence. But because members are genuinely initialized in declaration order, a list written out
of order can also hide a real bug: if one member's initializer reads another member that's declared
*later*, the value it reads is indeterminate (that member's own storage hasn't been given a value yet),
and reading an indeterminate scalar value is undefined behaviour - the same underlying problem as reading
any other uninitialized variable, just harder to spot because the initializer list makes it look like the
source was already set up. cppcheck doesn't distinguish the two cases - it always suggests reordering the
list to match declaration order, which prevents the harmful case from ever being possible.

## How to fix

Write the initializer list in the same order the members are declared in the class.

Before:
```cpp
class Fred {
    int a, b, c;
public:
    Fred() : c(0), b(0), a(0) { } // <- listed in the reverse of declaration order
};
```

After:
```cpp
class Fred {
    int a, b, c;
public:
    Fred() : a(0), b(0), c(0) { }
};
```

Before:
```cpp
class Foo {
public:
    Foo(int arg) : a(b), b(arg) {} // <- 'a' is initialized from 'b', but 'b' isn't set yet
    int a;
    int b;
};
```

After:
```cpp
class Foo {
public:
    explicit Foo(int arg) : a(arg), b(arg) {}
    int a;
    int b;
};
```

## Related checkers

- [useInitializationList.md](useInitializationList.md) - a different constructor-initializer-list
  pitfall, about whether the list is used at all rather than the order members appear in it.
- [selfInitialization.md](selfInitialization.md) - the more specific case where a member is initialized
  from itself.
