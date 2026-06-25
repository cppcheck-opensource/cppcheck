# functionConst

**Message**: Technically the member function 'x' can be const<br/>
**Category**: Robustness<br/>
**Severity**: Style (Inconclusive)<br/>
**Language**: C++

## Description

This checker identifies member functions that do not modify any member variables and therefore could be declared as `const`. A const member function promises not to modify the object's state and enables the function to be called on const objects.

The danger is that a const member function is allowed to have side effects outside the object. If you create a member function that formats the hard drive using
system calls it might be technically possible to make the function const, but is it "correct"? Using a const object is not supposed to have side effects.

For methods that has no side effects whatsoever; making them const is recommended.

The checker analyzes member functions and detects when:
- The function only reads member variables (never writes to them)
- The function only calls other const member functions
- The function only modifies parameters passed by reference or pointer (not member variables)
- The function performs operations that don't change the object's logical state

This warning is marked as **inconclusive** because while the function can technically be made const, it may not always be "correct".

This warning helps improve code quality by:
- Making the function's non-modifying nature explicit
- Enabling the function to be called on const objects
- Improving const-correctness throughout the codebase
- Helping with compiler optimizations
- Making code intentions clearer to other developers

## Motivation

The motivation of this checker is to improve robustness by making the code more const-correct.

## How to fix

Add the `const` keyword after the function signature to indicate that the function does not modify the object's state.

Before:
```cpp
class Rectangle {
    int width, height;
public:
    int getWidth() { return width; }
    int getHeight() { return height; }
    int getArea() { return width * height; }

    void printInfo() {
        std::cout << "Width: " << width << ", Height: " << height << std::endl;
    }

    bool isSquare() {
        return width == height;
    }

    void copyDataTo(Rectangle& other) {
        other.width = width;
        other.height = height;
    }
};
```

After:
```cpp
class Rectangle {
    int width, height;
public:
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    int getArea() const { return width * height; }

    void printInfo() const {
        std::cout << "Width: " << width << ", Height: " << height << std::endl;
    }

    bool isSquare() const {
        return width == height;
    }

    void copyDataTo(Rectangle& other) const {
        other.width = width;
        other.height = height;
    }
};
```

## Related checkers

- `functionStatic` - for member functions that can be declared static
- `constParameter` - for function parameters that can be const
- `constParameterReference` - for reference parameters that can be const
- `constParameterPointer` - for pointer parameters that can be const
- `constVariable` - for local variables that can be const
- `constVariableReference` - for local reference variables that can be const

## Notes

- This check is marked as **inconclusive** because the decision to make a function const should also consider the conceptual design
- Virtual functions should be carefully considered before making them const, as this affects the entire inheritance hierarchy
- Think about whether the function's purpose is to query state (should be const) or to perform an action (may not need to be const)
