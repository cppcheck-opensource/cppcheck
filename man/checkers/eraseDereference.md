# eraseDereference

**Message**: Iterator 'iter' used after element has been erased.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C++

## Description

An iterator is dereferenced (or compared) after the element it pointed to has already been erased.

## Motivation

Once the element an iterator points to has been erased, the iterator itself is invalid - dereferencing
or comparing it afterwards is undefined behaviour, even though it often still appears to "work" because
the underlying memory hasn't been reused yet.

## How to fix

Before:
```cpp
#include <map>
#include <iostream>
void f() {
    std::map<int, int> ints;
    std::map<int, int>::iterator iter;
    iter = ints.begin();
    ints.erase(iter);
    std::cout << iter->first << std::endl; // <- eraseDereference: 'iter' was just erased
}
```

After:
```cpp
#include <map>
#include <iostream>
void f() {
    std::map<int, int> ints = {{1, 2}};
    std::map<int, int>::iterator iter;
    iter = ints.begin();
    std::cout << iter->first << std::endl;
    ints.erase(iter);
}
```

## Related checkers

- [invalidIterator1.md](invalidIterator1.md) - a related mistake where an already-erased iterator is
  passed to `erase()`/`insert()` again, rather than dereferenced.
- [derefInvalidIterator.md](derefInvalidIterator.md) - the more general check for dereferencing an
  iterator that could be invalid for any reason, not specifically because it was just erased.
