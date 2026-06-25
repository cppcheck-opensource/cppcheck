
# preprocessorErrorDirective

**Message**: #error message<br/>
**Category**: Configuration<br/>
**Severity**: Error<br/>
**Language**: C and C++

## Description

The `#error` directive is a preprocessor instruction in C and C++ that explicitly generates a compilation error. It is typically used as a safeguard to prevent compilation under incorrect conditions—like unsupported configurations, platforms, or missing defines.

These warnings from Cppcheck do not indicate a bug in your code. These warnings indicate that the Cppcheck configuration is not working.

## How to fix

The warning is typically reported for an `#error` directive that is located inside some conditional preprocessor block (`#if..`, `#else`, etc):
```cpp
#ifndef __BYTE_ORDER__
#error Byte order is not defined
#endif
```

The code here is correct and you should not try to change it.

Somehow it will be necessary to define `__BYTE_ORDER__` in Cppcheck analysis.

### gcc compiler macro
If you compile your code with gcc and the macro is provided by gcc, then you can define all gcc-macros using these commands:
```
echo x > dummy.c
gcc -dM -E dummy.c > gcc-macros.h
```
The gcc-macros.h that is generated can be included in cppcheck using the `--include` option:
```
cppcheck --include=gcc-macros.h  ....
```

### library macro
If the macro that is needed is defined in some library header it might be possible to fix the issue by using an extra `--library` option:
```
cppcheck --library=foo .....
```

### manually defined macro
To define extra macros manually you can use `-D`:
```
cppcheck -D__BYTE_ORDER__=123 .....
```

### use --force or --max-configs
You can let Cppcheck try to resolve the required defines:
```
cppcheck --force .....
```
