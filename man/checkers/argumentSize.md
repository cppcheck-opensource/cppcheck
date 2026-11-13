# argumentSize

**Message**: Buffer 'a' is too small, the function 'f' expects a bigger buffer in 1st argument<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

A function is declared with a fixed-size array parameter (`void f(char a[10])`), and is called with an
array that's known to be smaller than that.

## Motivation

A parameter declared as `char a[10]` documents (and, inside the function, is used as if) an array of at
least 10 elements. Calling it with a genuinely smaller array is passed silently - the function has no
way to know the actual array it received was too small. cppcheck only compares the declared parameter
size against the size of the array actually passed - it doesn't check whether the function's body goes
on to access an element near the end of the declared size. If it does, as it's entitled to assume it
can, that access reads or writes past the real (smaller) array, which is undefined behaviour; if the
function only ever touches the first few elements in practice, this particular call happens to be
harmless despite the size mismatch.

## How to fix

Before:
```cpp
void f(char a[10]);
void g() {
    char a[2];
    f(a); // <- 'a' is smaller than what f() expects
}
```

After:
```cpp
void f(char a[10]);
void g() {
    char a[10];
    f(a);
}
```

## Related checkers

- [ctuArrayIndex.md](ctuArrayIndex.md) - a related, whole-program check for an out-of-bounds access
  reached through a function argument, including via a plain pointer parameter.
