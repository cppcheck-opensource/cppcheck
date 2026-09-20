# invalidFree

**Message**: Mismatching address is not returned from malloc(). The address you get from malloc() must be freed without offset.<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C/C++

## Description

`free()`/`delete`/`delete[]` is called on a pointer that has been moved by pointer arithmetic since it
was allocated - the allocator only recognizes the exact address it originally returned.

## Motivation

Freeing an address other than the exact one an allocator returned is undefined behaviour - typically a
crash, but potentially memory corruption that surfaces somewhere else entirely. This is easy to
introduce by accident once a pointer is advanced (for example while parsing or iterating through a
buffer) and then freed without first restoring it to the original address. cppcheck only tracks a
pointer as long as it can be sure nothing else changed it; passing it to another function is enough
uncertainty that cppcheck stops checking it rather than guess, so this catches only the mismatches it
can prove, not every one that exists.

## How to fix

Free the exact pointer the allocator returned, not one that's been offset since.

Before:
```cpp
#include <cstdlib>
void foo() {
  char *a; a = malloc(1024);
  free(a + 10); // <- this isn't the address malloc() returned
}
```

After:
```cpp
#include <cstdlib>
void foo() {
  char *a; a = malloc(1024);
  free(a);
}
```
