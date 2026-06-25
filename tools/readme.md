## Cppcheck developer and build tools

### * tools/matchcompiler.py

The matchcompiler.py is a build script that performs a few code transformations to *.cpp* files under the *lib* directory. These transformations are related to the use of `Token::Match()` function and are intended to improve code performance. The transformed files are saved on the *build* directory. This tool is silently used when building the code with `SRCDIR=build`, that is:
```shell
$ cd path/to/cppcheck
$ make MATCHCOMPILER=yes
```
Here is a simple example of the *matchcompiler.py* optimization. Suppose there is a file *example.cpp* under *lib/*:
```cpp
// lib/example.cpp
void f1() {
    Token::Match(tok, "abc");
}

void f2() {
    const char *abc = "abc";
    Token::Match(tok, abc);
}
```
If you manually run *matchcompiler.py* from the main directory:
```shell
$ cd path/to/cppcheck
$ python tools/matchcompiler.py
```
A file *example.cpp* will be generated on the *build* directory:
```cpp
// build/example.cpp
#include "token.h"
#include "errorlogger.h"
#include <string>
#include <cstring>
static const std::string matchStr1("abc");
// pattern: abc
static bool match1(const Token* tok) {
    if (!tok || !(tok->str()==matchStr1)/* abc */)
        return false;
    return true;
}
void f1() {
    match1(tok);
}

void f2() {
    const char *abc = "abc";
    Token::Match(tok, abc);
}
```
From this we can see that the usage of `Token::Match()` in `f1()` has been optimized, whereas the one in `f2()` couldn't be optimized (the string wasn't inline on the `Token::Match()` call). **The developer doesn't need to use this tool during development but should be aware of these optimizations**. *Building with this optimization, cppcheck can get a boost of 2x of speed-up.*

### * tools/dmake/dmake.cpp

Automatically generates the main `Makefile` for Cppcheck (**the main `Makefile` should not be modified manually**). To build and run the `dmake` tool execute:
```shell
$ cd path/to/cppcheck
$ make dmake
$ ./dmake
```
### * tools/reduce.py

Script that reduces code for a hang/false positive.

### * tools/times.sh

Script to generate a `times.log` file that contains timing information of the last 20 revisions.

### * tools/donate-cpu.py

Script to donate CPU time to Cppcheck project by checking current Debian packages.

### * tools/test-my-pr.py

Script to compare result of working Cppcheck from your branch with main branch.

### * tools/triage

This tool lets you comfortably look at Cppcheck analysis results for daca packages. It automatically
downloads the package, extracts it and jumps to the corresponding source code for a Cppcheck
message.

### * tools/check-errorids.sh

Script to compare the error IDs in the expected `testrunner` output (without executing it) with the `--errorlist` output.
It will report missing test coverage for an ID and missing IDs in the `--errorlist` output.
