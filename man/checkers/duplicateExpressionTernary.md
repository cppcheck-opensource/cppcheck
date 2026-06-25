# duplicateExpressionTernary

**Message**: Same expression in both branches of ternary operator<br/>
**Category**: Logic<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

This checker detects when syntactically identical expressions appear in both the true and false branches of a ternary operator (`condition ? true_expr : false_expr`).

The warning is triggered when:
- The second and third operands of a ternary operator are syntactically identical expressions
- This includes cases where variables are referenced through aliases that resolve to the same expression

## Motivation

The same expression indicates that there might be some logic error or copy-paste mistake.

## Examples

### Problematic code

```cpp
// Same expression in both branches
int result = condition ? x : x;  // Warning: duplicateExpressionTernary

// Same variable referenced through alias
const int c = a;
int result = condition ? a : c;  // Warning: duplicateExpressionTernary
```

### Fixed code

```cpp
// Different expressions in branches
int result = condition ? x : y;  // OK
```

## How to fix

1. **Check for copy-paste errors**: Verify that both branches are supposed to have the same expression
2. **Use different expressions**: If the branches should differ, update one of them
3. **Simplify the code**: If both branches are intentionally the same, remove the ternary operator entirely

Before:
```cpp
int getValue(bool flag) {
    return flag ? computeValue() : computeValue();  // Same computation in both branches
}
```

After:
```cpp
int getValue(bool flag) {
    return computeValue();  // Simplified - condition doesn't matter
}
```

Or if the branches should differ:
```cpp
int getValue(bool flag) {
    return flag ? computeValue() : computeAlternativeValue();  // Different computations
}
```
