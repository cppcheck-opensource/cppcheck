# constParameterPointer

**Message**: Parameter 'x' can be declared as pointer to const<br/>
**Category**: Robustness<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

This checker identifies function parameters that are pointers which are never used to modify the data they point to. Such parameters can be declared as "pointer to const" to improve code clarity and prevent accidental modifications.

The checker analyzes pointer parameters and detects when:
- The pointer is dereferenced only for reading values (not writing)
- The pointer is used in comparisons or logical operations
- The pointer is passed to functions that expect const pointers
- The pointer arithmetic is performed without modifying the pointed-to data

This warning helps improve code quality by:
- Making the intent clear that the function will not modify the pointed-to data
- Enabling compiler optimizations
- Preventing accidental modifications
- Improving const-correctness

## Motivation

This checker improves const-correctness. Const-correct code is more robust and easier to understand.

## How to fix

Add the `const` keyword to the parameter declaration to indicate that the pointed-to data will not be modified.

Before:
```cpp
void printValue(int* p) {
    printf("%d\n", *p);  // Only reading the value
}

int findMax(int* arr, size_t size) {
    int max = arr[0];
    for (size_t i = 1; i < size; i++) {
        if (arr[i] > max) {  // Only reading array elements
            max = arr[i];
        }
    }
    return max;
}

bool isEqual(char* str1, char* str2) {
    return strcmp(str1, str2) == 0;  // Only reading strings
}
```

After:
```cpp
void printValue(const int* p) {
    printf("%d\n", *p);  // Clearly indicates read-only access
}

int findMax(const int* arr, size_t size) {
    int max = arr[0];
    for (size_t i = 1; i < size; i++) {
        if (arr[i] > max) {
            max = arr[i];
        }
    }
    return max;
}

bool isEqual(const char* str1, const char* str2) {
    return strcmp(str1, str2) == 0;  // Standard library functions expect const char*
}
```

## Related checkers

- `constParameter` - for non-pointer parameters that can be const
- `constParameterReference` - for reference parameters that can be const
- `constParameterCallback` - for callback function parameters that can be const
- `constVariablePointer` - for local pointer variables that can be const

## Notes

- This check is disabled for virtual functions and callback functions where changing the signature might break polymorphism or function pointer compatibility
- The checker may suggest callback-specific warnings when a function is used as a callback, indicating that const-ification might require casting function pointers
- Template functions are handled carefully to avoid false positives in generic code