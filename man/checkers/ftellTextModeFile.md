# ftellModeTextFile 

**Message**: The ftell function obtains the current value of the file position indicator for the stream pointed to by stream. For a binary stream, the value is the number of characters from the beginning of the file. For a text stream, its file position indicator contains unspecified information, usable by the fseek function for returning the file position indicator for the stream to its position at the time of the ftell call; the difference between two such return values is not necessarily a meaningful measure of the number of characters written or read. See a7.21.9.4 in C11 standard.<br/>
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

        position = fseek(f, 0, SEEK_END);
        fstat(f, &st);
        printf( "Position  %d\n", ftell(f);
        printf( "File size %d\n, st.st_size);
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

