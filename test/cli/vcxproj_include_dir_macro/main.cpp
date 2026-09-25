#include "myheader.h"
#ifndef FROM_REAL_HEADER
#error myheader.h (under RealInc, reached only via the unresolved AdditionalIncludeDirectories macro) was not found
#endif
int main() { return 0; }
