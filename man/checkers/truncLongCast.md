# truncLongCastAssignment and truncLongCastReturn

**Message**: int result is assigned to long variable. If the variable is long to avoid loss of information, then you have loss of information.<br/>
**Category**: Type Safety<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

This checker warns when a calculation has type 'int' and it could potentially overflow that and the result is implicitly or explicitly converted to a larger
integer type after the loss of information has already occurred.

## Motivation

The motivation of this checker is to catch bugs. Unintentional loss of information.

## How to fix

You can fix these warnings by:
1. Add explicit cast to avoid loss of information
2. Explicitly truncate the result
3. Change type of assigned variable

Before:
```cpp
void foo(int32_t y) {
    int64_t x = y * y; // <- warning
}
```

After (explicit cast):
```cpp
void foo(int32_t y) {
    int64_t x = (int64_t)y * y; // <- 64-bit multiplication
}
```

After (explicitly truncate the result):
```cpp
void foo(int32_t y) {
    int64_t x = (int32_t)(y * y); // redundant cast makes it explicit that your intention is to truncate the result to 32-bit
}
```

After (change type of assigned variable):
```cpp
void foo(int32_t y) {
    int32_t x = y * y;
}
```