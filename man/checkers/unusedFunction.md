# unusedFunction and staticFunction

**Message**: The function 'foo' is never used.<br/>
**Category**: Unused Code<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

- `unusedFunction`: a function is defined but never called (and its address is never taken) anywhere
  in the code cppcheck was given.
- `staticFunction` (C only): a function is only ever called from within its own file, and could be
  given internal linkage by declaring it `static`.

Both are only enabled through `--enable=unusedFunction` - not through `--enable=style`, even though
the messages are reported at `style` severity - and cppcheck explicitly recommends only enabling this
check when the whole program (every source file of the project) is being analyzed together, not a
single file in isolation.

Both only consider a function "used" through a direct, by-name call (or its address being taken)
somewhere in that whole-program analysis. A virtual function is never checked, since it may only ever
be reached polymorphically through a base-class pointer/reference, which cppcheck doesn't attempt to
trace; operator overloads are likewise never checked, since they're often invoked implicitly through
operator syntax or required by generic code, making usage hard to establish reliably by name alone.

## Motivation

An unused function is dead code: it adds to what has to be read and maintained without doing
anything. For `staticFunction`, giving an internal-only function `static` linkage documents that it
is not part of the file's public interface, and can also help the compiler optimize it.

## How to fix

Remove a function that's genuinely unused, or make sure the code that calls it is included in the
same whole-program analysis. Add `static` to a C function that's only called from its own file.

Before:
```cpp
void helper() { /* ... */ } // <- unusedFunction: never called anywhere in the analyzed code

int main() {
    return 0;
}
```

After:
```cpp
void helper() { /* ... */ }

int main() {
    helper();
    return 0;
}
```

Before (C):
```c
void helper(void) { /* ... */ } // <- staticFunction: only called from this file

int main(void) {
    helper();
    return 0;
}
```

After:
```c
static void helper(void) { /* ... */ }

int main(void) {
    helper();
    return 0;
}
```

