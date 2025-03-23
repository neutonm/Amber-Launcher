#include <commands/regedit.h>

#include <core/command.h>
#include <core/opsys.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

CAPI void 
SCommand_Callback_RegEdit(const SCommand* pSelf, const SCommandArg* pArg)
{
    UNUSED(pSelf);
    UNUSED(pArg);

    AmberLauncher_SetRegistryKey("2dacceloff",      0);
    AmberLauncher_SetRegistryKey("3DSoundAvailable",1);
    AmberLauncher_SetRegistryKey("D3DAvailable",    1);
    AmberLauncher_SetRegistryKey("Colored Lights",  0);
    AmberLauncher_SetRegistryKey("Use D3D",         1);
    AmberLauncher_SetRegistryKey("Disable3DSound",  0);
    AmberLauncher_SetRegistryKey("startinwindow",   0);
}

CAPI int 
LUA_SetRegistryKey(lua_State *L)
{
    const char      *sKey   = luaL_checkstring(L, 1);
    const uint32    dValue  = (uint32)luaL_checkinteger(L, 2);

    if (AmberLauncher_SetRegistryKey(sKey, dValue) != 0)
    {
        return luaL_error(L, "AmberLauncher_SetRegistryKey failed to execute properly");
    }

    return 0;
}

CAPI int 
LUA_GetRegistryKey(lua_State *L)
{
    const char *sKey   = luaL_checkstring(L, 1);
    uint32      dValue = 0;

    if (AmberLauncher_GetRegistryKey(sKey, &dValue) != 0)
    {
        return luaL_error(L, "AmberLauncher_GetRegistryKey failed to execute properly");
    }

    lua_pushinteger(L, dValue);
    return 1;
}

