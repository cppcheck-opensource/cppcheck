# functionStatic

**Message**: The member function 'x' can be static<br/>
**Category**: Readability<br/>
**Severity**: Style<br/>
**Language**: C++

## Description

This checker identifies member functions that do not access any non-static member variables or call any non-static member functions. Such functions can be declared as `static` to indicate that they don't require an object instance to operate.

This warning helps improve code quality by:
- Making the function's independence from object state explicit
- Enabling the function to be called without creating an object instance
- Clarifying the function's scope and dependencies

## Motivation

The motivation of this checker is to improve readability.

## How to fix

Add the `static` keyword to the function declaration to indicate that it doesn't require an object instance.

Before:
```cpp
class Calculator {
public:
    int add(int a, int b) {
        return a + b;  // Only uses parameters
    }

    void printMessage() {
        std::cout << "Hello World" << std::endl;  // Uses no instance data
    }

    bool isValidNumber(int num) {
        return num > 0 && num < 1000;  // Pure function
    }
};
```

After:
```cpp
class Calculator {
public:
    static int add(int a, int b) {
        return a + b;  // Can be called as Calculator::add(5, 3)
    }

    static void printMessage() {
        std::cout << "Hello World" << std::endl;  // Can be called without instance
    }

    static bool isValidNumber(int num) {
        return num > 0 && num < 1000;  // Clearly indicates no state dependency
    }
};
```

## Related checkers

- `functionConst` - for member functions that can be declared const

