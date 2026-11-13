# useStlAlgorithm

**Message**: Consider using std::any_of algorithm instead of a raw loop.<br/>
**Category**: Code Quality<br/>
**Severity**: Style<br/>
**Language**: C++

## Description

A hand-written loop follows a shape (accumulate a value, count matches, find the first/last match, copy
matching elements, ...) that a single call to a standard `<algorithm>` function would do more clearly
and with less room for an off-by-one mistake.

## Motivation

A raw loop that reimplements a standard algorithm is more code to read and more room for a subtle bug
(an off-by-one, a missed edge case) than calling the algorithm that already exists for exactly this
purpose. Naming the intent directly (`std::any_of`, `std::count_if`, `std::find_if`, `std::copy_if`,
`std::transform`, ...) also tells a reader what the loop is *for* without having to trace through its
body.

## How to fix

Before:
```cpp
#include <vector>
bool f(bool b) {
  std::vector<int> v;
  if (b)
    v.push_back(0);
  for (auto i : v)      // <- consider std::any_of instead of this raw loop
    if (v[i] > 0)
      return true;
  return false;
}
```

After:
```cpp
#include <vector>
#include <algorithm>
bool f(bool b) {
  std::vector<int> v;
  if (b)
    v.push_back(0);
  return std::any_of(v.begin(), v.end(), [](int i){ return v[i] > 0; });
}
```

