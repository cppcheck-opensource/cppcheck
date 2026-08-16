# wrongfeofUsage

**Message**: Using feof() as a loop condition causes the last line to be processed twice.<br/>
**Category**: Correctness<br/>
**Severity**: Warning<br/>
**Language**: C/C++

## Description

`feof()` returns non-zero only after a read operation has failed because the end of file was reached. When used as the sole condition of a loop, the loop body executes one extra time after the last successful read: the read fails silently (or returns partial data), and only then does `feof()` return true and terminate the loop.

This checker matches `while (!feof(fp))` and `do { ... } while (!feof(fp))` loops and warns when all of the following are true:

- The loop body contains at least one file-read call (`fgets`, `fgetc`, `getc`, `fread`, or `fscanf`) on the same file pointer.
- The destination (return value or output buffer) of the **last** file-read call in the loop is used after that call within the same loop iteration.

The checker skips loops that contain a control-flow statement (`return`, `break`, `goto`, `continue`, `throw`) as those are too complex to analyze reliably, and loops where the file pointer appears in a context other than a recognised I/O function (`fgets`, `fgetc`, `getc`, `fread`, `fscanf`, `fprintf`, `fwrite`, `fputs`, `fputc`, `putc`).

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