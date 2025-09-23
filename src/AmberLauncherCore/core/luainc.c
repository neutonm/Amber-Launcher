#include <core/luainc.h>

/******************************************************************************
 * HEADER FUNCTION DEFINITIONS
 ******************************************************************************/
#if LUA_VERSION_NUM == 501
void
luaL_setfuncs(lua_State *L, const luaL_Reg *l, int nup)
{
    (void)nup;
    for (; l->name != NULL; l++) {
        lua_pushcfunction(L, l->func);
        lua_setfield(L, -2, l->name);
    }
}

void
luaL_traceback(lua_State *L, lua_State *L1, const char *msg, int level)
{
    if (L != L1)
        lua_pushthread(L1);

    lua_getglobal(L1, "debug");
    if (!lua_istable(L1, -1))
    {
        lua_pop(L1, 1);
        lua_pushliteral(L, "debug library not available");
        return;
    }
    lua_getfield(L1, -1, "traceback");
    if (!lua_isfunction(L1, -1)) 
    {
        lua_pop(L1, 2);
        lua_pushliteral(L, "debug.traceback not available");
        return;
    }

    if (msg)
    {
        lua_pushstring(L1, msg);
    }
    else
    {
        lua_pushnil(L1);
    }
    lua_pushinteger(L1, level);

    int argcount = (L != L1) ? 3 : 2;
    lua_call(L1, argcount, 1);

    lua_xmove(L1, L, 1);
    lua_pop(L1, 1);
}
#endif
