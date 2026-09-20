# constParameterCallback

**Message**: Parameter 'x' can be declared with const, however it seems that 'f' is a callback function.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

Same idea as [constParameter.md](constParameter.md)/[constParameterReference.md](constParameterReference.md)/
[constParameterPointer.md](constParameterPointer.md): a parameter is never used to modify what it refers
to, so it could be declared `const`. This variant is for when the function is used as a callback (passed
as a function pointer) - fixing it may also require adjusting whatever calls through that function
pointer.

## Motivation

A missing `const` hides a guarantee the compiler could otherwise enforce and readers could otherwise
rely on. The callback case is called out separately because the fix isn't purely local: the function
pointer type it's assigned to also needs to change, or the cast at the call site needs adjusting.

## How to fix

Before:
```cpp
#include <vector>
void dostuff(int (*cb)(std::vector<int>&));
int callback(std::vector<int>& x) { return x[0]; } // <- 'x' is only read
void f() { dostuff(callback); }
```

After: const-ify the parameter, and the function pointer type it's passed through.
```cpp
#include <vector>
void dostuff(int (*cb)(const std::vector<int>&));
int callback(const std::vector<int>& x) { return x[0]; }
void f() { dostuff(callback); }
```

## Related checkers

- [constParameter.md](constParameter.md), [constParameterReference.md](constParameterReference.md),
  [constParameterPointer.md](constParameterPointer.md) - the same idea, for a parameter that isn't part
  of a callback function's signature.
