# invalidContainer and invalidContainerReference

**Message**: Using pointer to local variable 'v' that may be invalid.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C++

## Description

A pointer or reference taken from inside a container (an element, `.front()`, an iterator, ...) is used
after a call (`push_back()`, `insert()`, `clear()`, ...) that may have invalidated it.

- `invalidContainer`: the invalidated thing is a pointer or iterator value.
- `invalidContainerReference`: the invalidated thing is a reference.

## Motivation

Many container operations can reallocate or otherwise invalidate previously-obtained pointers,
references, and iterators into that container's data, without any visible change at the call site of
the operation that invalidates them. Using one afterwards is undefined behaviour.

## How to fix

Before:
```cpp
#include <vector>
#include <iostream>
void f(std::vector<int> &v) {
    int *v0 = &v[0];
    v.push_back(123);
    std::cout << *v0 << std::endl; // <- invalidContainer: push_back() may have reallocated 'v'
}
```

After:
```cpp
#include <vector>
#include <iostream>
void f(std::vector<int> &v) {
    v.push_back(123);
    std::cout << v[0] << std::endl;
}
```

Before:
```cpp
#include <vector>
#include <iostream>
void f() {
    std::vector<int> v = {1};
    int &v0 = v.front();
    v.push_back(123);
    std::cout << v0 << std::endl; // <- invalidContainerReference: push_back() may have reallocated 'v'
}
```

After:
```cpp
#include <vector>
#include <iostream>
void f() {
    std::vector<int> v = {1};
    v.push_back(123);
    std::cout << v.front() << std::endl;
}
```

## Related checkers

- [invalidContainerLoop.md](invalidContainerLoop.md) - the equivalent problem when the invalidating
  call happens while a loop is actively iterating over the same container.
