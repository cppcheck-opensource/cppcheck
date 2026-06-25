
// Test library configuration for lua.cfg
//
// Usage:
// $ cppcheck --check-library --library=lua --enable=style,information --inconclusive --error-exitcode=1 --inline-suppr test/cfg/lua.c
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

#include <lua.h>
#include <stdio.h>

void validCode(lua_State *L)
{
    int a = lua_gettop(L);
    printf("%d", a);
    lua_pushnil(L);
    lua_pop(L, 1);
}

void ignoredReturnValue(lua_State *L)
{
    // cppcheck-suppress ignoredReturnValue
    lua_tonumber(L, 1);
    // cppcheck-suppress ignoredReturnValue
    lua_tostring(L, 1);
    // cppcheck-suppress ignoredReturnValue
    lua_isboolean(L, 1);
    // cppcheck-suppress ignoredReturnValue
    lua_isnil(L, 1);
}
