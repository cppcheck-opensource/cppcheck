# suspiciousSemicolon

**Message**: Suspicious use of ; at the end of 'if' statement.<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

A stray `;` immediately follows an `if`/`for`/`while` condition, right before a `{ ... }` block - the
block is then unconditional, but reads as if it were controlled by the condition.

## Motivation

`if (cond);` is a complete statement on its own (an empty statement, executed only when `cond` is true),
so a `{ ... }` block written right after it is a separate, always-executed statement - not the body of
the `if` at all. This is a classic typo that silently turns a conditional block into unconditional code,
and it's easy to miss on a quick read since the block still looks indented as if it belonged to the
`if`.

## How to fix

Remove the stray semicolon.

Before:
```cpp
void do_something();
void foo(bool quit) {
  while (!quit); { // <- the loop body is just ';' - this block always runs once
    do_something();
  }
}
```

After:
```cpp
void do_something();
void foo(bool quit) {
  while (!quit) {
    do_something();
  }
}
```
