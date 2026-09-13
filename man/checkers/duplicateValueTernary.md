# duplicateValueTernary

**Message**: Same value in both branches of ternary operator<br/>
**Category**: Logic<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

This checker detects when both branches of a ternary operator (`condition ? true_expr : false_expr`) evaluate to the same value, even though the expressions themselves may be syntactically different.

The warning is triggered when:
- The second and third operands of a ternary operator evaluate to the same constant value
- The expressions are constant statements that don't change during evaluation
- The expressions have different syntax but equivalent values

However, no warning is generated when:
- The expressions have different values on different platforms
- Variables are modified between evaluations
- The expressions involve non-constant computations

## Motivation

The same value indicates that there might be some logic error or copy-paste mistake.

Note that a ternary whose branches merely *could* differ depending on the platform, such as
`is_64bit ? sizeof(long) : sizeof(int)`, is not flagged: cppcheck only warns when it can tell both
branches are the same value on the platform being analyzed.

## How to fix

1. **Check for logic errors**: Verify if both branches should actually have the same value
2. **Simplify the code**: If both branches always have the same value, remove the ternary operator
3. **Use different values**: If the branches should differ, update one of them
4. **Remove unnecessary casts**: If the difference is only in casting, consider if the cast is needed

Before:
```cpp
int getValue(bool flag) {
    return flag ? (int)42 : 42;  // Same value, different expressions
}
```

After (simplified):
```cpp
int getValue(bool flag) {
    return 42;  // Simplified - condition doesn't affect result
}
```

Or (if branches should differ):
```cpp
int getValue(bool flag) {
    return flag ? 42 : 0;  // Different values for different conditions
}
```
