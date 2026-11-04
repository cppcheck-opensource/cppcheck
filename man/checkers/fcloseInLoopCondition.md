# fcloseInLoopCondition

**Message**: fclose() used as loop condition may skip loop body or double-close file handle.<br/>
**Category**: Resource Management<br/>
**Severity**: Warning<br/>
**Language**: C and C++

## Description

Using `fclose()` as a loop condition leads to two unwanted outcomes:

- On **success**, the condition is false and the loop body never executes. The intent was likely to process the file inside the loop, but it is already closed.
- On **failure**, the condition is true and the loop body executes but `fclose()` is called again on the already-closed file handle on the next iteration, which is undefined behaviour.

This pattern is almost always a misunderstanding of what `fclose()` returns, or confusion with a function that reads/processes data incrementally (like `fgets` or `fread`). Unlike those functions, `fclose()` is a one-shot teardown operation and has no meaningful retry or loop-until-done semantic.

## How to fix

Call `fclose()` outside the loop condition. If you need to check whether the close succeeded, store the return value and test it separately.

Before:
```c
FILE *fp = fopen("data.txt", "r");
while (fclose(fp)) {
    /* process file */
}
```

After:
```c
FILE *fp = fopen("data.txt", "r");
/* process file */
if (fclose(fp) != 0) {
    /* handle close error */
}
```

## Related checkers

- `useClosedFile` - for using a file handle that has already been closed
