# wrongfeofUsage

**Message**: Using feof() as a loop condition causes the last line to be processed twice.<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

`feof()` returns non-zero only after a read operation has failed because the end of file was reached. When used as the sole condition of a `while` loop, the loop body executes one extra time after the last successful read: the read fails silently (or returns partial data), and only then does `feof()` return true and terminate the loop.

This checker warns when it finds feof in the loop condition and either:
- no file-read call (e.g. `fgets`, `fread`, `fscanf`) precedes the loop and is also present inside the loop body
- a control-flow statement (`return`, `break`, `goto`, `continue`, `throw`) is present in the the loop body.

## How to fix

Check the return value of the read function directly in the loop condition.

Before:
```c
void process(FILE *fp) {
    char line[256];
    while (!feof(fp)) {          /* wrong: processes last line twice */
        fgets(line, sizeof(line), fp);
        puts(line);
    }
}
```

After:
```c
void process(FILE *fp) {
    char line[256];
    while (fgets(line, sizeof(line), fp) != NULL) {
        puts(line);
    }
}
```