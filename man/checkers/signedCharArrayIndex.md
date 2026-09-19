# signedCharArrayIndex and unknownSignCharArrayIndex

**Message**: Signed 'char' type used as array index.<br/>
**Category**: Undefined Behaviour/Portability<br/>
**Severity**: Warning/Portability<br/>
**Language**: C/C++

## Description

- `signedCharArrayIndex` (warning): an array is indexed with a `char` known to be signed, and the index
  can be 128 or more - once sign-extended, a value like `200` becomes negative, so the access lands
  before the start of the array instead of near its expected position. This ID specifically covers the
  case where cppcheck knows the index is a signed `char` that *can* reach 128 or more, but can't work
  out its exact wrapped-around value; when the concrete wrapped value (or a condition it depends on) is
  known, the same bug is instead reported as the more specific [negativeIndex.md](negativeIndex.md) or
  [arrayIndexOutOfBounds.md](arrayIndexOutOfBounds.md), which is what happens in most real code.
- `unknownSignCharArrayIndex` (portability): the same situation, but for a plain `char` whose signedness
  the C/C++ standard leaves up to the compiler - the exact same source code indexes the array
  differently depending on which platform/compiler it's built with.

## Motivation

On many platforms `char` can hold negative values (typically -128..127 instead of 0..255). A byte value
of 128 or more, once treated as a signed `char`, becomes negative - so using it directly as an array
index produces a negative index instead of the expected large-but-positive one, reading or writing
before the start of the array.

## How to fix

Use `unsigned char` for a value that's meant to index into a table by its raw byte value.

Before:
```cpp
int buf[256];
void foo() {
    char ch = 0x80;
    buf[ch] = 0; // <- negative on platforms where 'char' is signed
}
```

After:
```cpp
int buf[256];
void foo() {
    unsigned char ch = 0x80;
    buf[ch] = 0;
}
```

## Related checkers

- [negativeIndex.md](negativeIndex.md) and [arrayIndexOutOfBounds.md](arrayIndexOutOfBounds.md) - the
  more specific IDs this checker's finding is usually superseded by.
- [charBitOp.md](charBitOp.md) - the same signed-`char` sign-extension pitfall, but for a bitwise
  operation instead of an array index.
