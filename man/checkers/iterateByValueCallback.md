# iterateByValueCallback

**Message**: Variable 'x' is used to iterate by value. It could be declared as a const reference which is usually faster and recommended in C++. However it seems that 'f' is a callback function.<br/>
**Category**: Code Quality<br/>
**Severity**: Performance<br/>
**Language**: C++

## Description

Same idea as [iterateByValue.md](iterateByValue.md): a range-based `for` loop copies each element
needlessly. This variant is for when the loop is inside a function used as a callback.

## Motivation

Copying every element of a container just to read it wastes time and memory proportional to the
element's size and the container's length, for no benefit over a `const` reference.

## How to fix

Declare the loop variable as a `const` reference instead of copying each element, the same as for
[iterateByValue.md](iterateByValue.md).

## Related checkers

- [iterateByValue.md](iterateByValue.md) - the same idea, for a loop that isn't inside a callback
  function.
- [passedByValueCallback.md](passedByValueCallback.md) - the same idea, for a callback's parameter
  instead of a loop variable.
