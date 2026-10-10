# unusedLabel, unusedLabelSwitch, unusedLabelConfiguration and unusedLabelSwitchConfiguration

**Message**: Label 'x' is not used.<br/>
**Category**: Code Quality<br/>
**Severity**: Style/Warning<br/>
**Language**: C/C++

## Description

A `goto` label is declared but no `goto` anywhere in the file jumps to it. The four IDs are the same
finding, refined by context:

- `unusedLabel`: the plain case - style severity.
- `unusedLabelSwitch`: the label sits directly where a `case` inside a `switch` was probably meant -
  warning severity, since this is a more likely sign of an actual typo.
- `unusedLabelConfiguration` / `unusedLabelSwitchConfiguration`: the same two situations, but the file
  also contains `#if`/`#ifdef`, so the missing `goto` might simply be in a preprocessor branch that
  wasn't analyzed this time.

## Motivation

An unused label is either dead code left over from a refactor, or - especially when it appears right
where a `case` would fit inside a `switch` - a typo for a different keyword entirely. Either way it's
worth a second look: removing genuinely dead labels keeps the code honest about what it does, and
catching a `case`/label typo can fix a real logic bug.

## How to fix

Either remove the label, or actually jump to it - or, if it was meant to be a `case`, fix the typo.

Before:
```cpp
void f() {
    label: // <- nothing 'goto's here
}
```

After: either remove the label, or actually jump to it.
```cpp
void f() {
    label:
    ;
    goto label;
}
```

Before:
```cpp
int test(char art) {
    switch (art) {
    caseZERO: // <- looks like a typo for 'case 0:'
        return 0;
    case 2:
        return 2;
    }
    return -1;
}
```

After:
```cpp
int test(char art) {
    switch (art) {
    case 0:
        return 0;
    case 2:
        return 2;
    }
    return -1;
}
```

Before:
```cpp
void f() {
#ifdef X
    goto END;
#endif
END: // <- only reachable through code hidden behind '#ifdef X'
    ;
}
```

After: keep the label's only `goto` in the same preprocessor branch as the label, or remove the label if
it's genuinely unused.
```cpp
void f() {
#ifdef X
    goto END;
END:
#endif
    ;
}
```
