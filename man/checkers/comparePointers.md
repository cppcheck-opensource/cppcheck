# comparePointers and subtractPointers

**Message**: Comparing pointers that point to different objects<br/>
**Category**: Undefined Behaviour<br/>
**Severity**: Error<br/>
**Language**: C++ only

## Description

Two pointers that cppcheck can see point into two different, unrelated variables or objects are
compared (`<`, `>`, `<=`, `>=` - `comparePointers`) or subtracted (`-` - `subtractPointers`). Only
pointers into the same array/object have a meaningful order or distance between them.

## Motivation

Comparing or subtracting pointers that don't point into the same array or object is undefined
behaviour: there's no guarantee about how unrelated objects are laid out in memory relative to each
other, so the result of `<`/`>`/`-` between them isn't meaningful, even though the code compiles and
often looks reasonable.

## How to fix

Only compare or subtract pointers that point into the same array or object.

Before:
```cpp
bool f() {
    int x = 0;
    int y = 0;
    int* xp = &x;
    int* yp = &y;
    return xp > yp; // <- comparePointers: 'x' and 'y' are unrelated variables
}
```

After:
```cpp
bool f() {
    int arr[2] = {0, 0};
    int* xp = &arr[0];
    int* yp = &arr[1];
    return xp > yp; // pointers into the same array can be meaningfully compared
}
```

Before:
```cpp
int f() {
    int x = 0;
    int y = 1;
    return &x - &y; // <- subtractPointers: the distance between unrelated variables is meaningless
}
```

After:
```cpp
int f() {
    int arr[2] = {0, 1};
    return &arr[0] - &arr[1];
}
```
