#include "common.h"

#ifndef COMMON_H_INCLUDED_MARKER
#error "common.h was not found - AdditionalIncludeDirectories from common.props did not resolve"
#endif

int main()
{
    int y = 2;
    return y / 0;
}
