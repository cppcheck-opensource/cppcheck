# ftellModeTextFile 

**Message**: ftell() result is unspecified when file is opened in mode "t".<br/>
**Category**: Portability<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

This checker detects the use of ftell() on a file open in text (or translate) mode. The text mode is not consistent
 in between Linux and Windows system and may cause ftell() to return the wrong offset inside a text file.

This warning helps improve code quality by:
- Making the intent clear that the use of ftell() in "t" mode may cause portability problem.

## Motivation

This checker improves portability accross system.

## How to fix

According to C11, the file must be opened in binary mode 'b' to prevent this problem.

Before:
```cpp
     FILE *f = fopen("Example.txt", "rt");
     if (f)
     {
        int position;
        struct stat st;

        // Wrong way to get the file size
        fseek(f, 0, SEEK_END);
        printf( "File size %d\n, ftell(f));
        fclose(f);
     }

```

After:
```cpp

     FILE *f = fopen("Example.txt", "rb");
     if (f)
     {
        fseek(f, 0, SEEK_END);
        printf( "Offset %d\n", ftell(f);
        fclose(f);
     }

```

## Notes

See https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/fopen-wfopen?view=msvc-170

