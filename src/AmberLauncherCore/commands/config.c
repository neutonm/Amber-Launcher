#include <commands/config.h>

#include <core/command.h>
#include <ext/ini.h>

#include <lua.h>
#include <lauxlib.h>

#include <stdio.h>

typedef struct lua_ini_t
{
    ini_t *pConfig;
} lua_ini_t;

CAPI CBOOL
SCommand_Callback_TweakConfig(const SCommand* pSelf, const SCommandArg* pArgs, const unsigned int dNumArgs)
{
    ini_t* config;
    const char *ViewDistanceD3D;

    UNUSED(pSelf);
    UNUSED(pArgs);
    UNUSED(dNumArgs);

    config = ini_load("mm7.ini");
    if (!config)
    {
        return CFALSE;
    }

    ViewDistanceD3D = ini_get(config, "Settings", "ViewDistanceD3D");
    if (ViewDistanceD3D)
    {
        fprintf(stdout, "ViewDistanceD3D: %s\n", ViewDistanceD3D);
    }

    ini_free(config);

    return CTRUE;
}

CAPI int
LUA_REGISTER_INIConfig(struct lua_State* L)
{
    luaL_newmetatable(L, "INIConfig");

    /* Set __gc metamethod */
    lua_pushstring(L, "__gc");
    lua_pushcfunction(L, LUA_INIClose);
    lua_settable(L, -3);

    /* Set __index metamethod for method access */
    lua_pushstring(L, "__index");
    lua_newtable(L);

    lua_pushstring(L, "get");
    lua_pushcfunction(L, LUA_INIGet);
    lua_settable(L, -3);

    lua_pushstring(L, "close");
    lua_pushcfunction(L, LUA_INIClose);
    lua_settable(L, -3);

    lua_settable(L, -3);

    lua_pop(L, 1);

    return 1;
}

CAPI int
LUA_INILoad(struct lua_State* L)
{
    const char* sFilename = luaL_checkstring(L, 1);
    ini_t *pConfig;
    lua_ini_t *pIniUserData;

    pConfig = ini_load(sFilename);
    if (!pConfig) 
    {
        lua_pushnil(L);
        return 1;
    }

    pIniUserData = (lua_ini_t *)lua_newuserdata(L, sizeof(lua_ini_t));
    pIniUserData->pConfig = pConfig;

    luaL_getmetatable(L, "INIConfig");
    lua_setmetatable(L, -2);

    return 1;
}

CAPI int
LUA_INIClose(struct lua_State* L)
{
    lua_ini_t* pIniUserData = (lua_ini_t*)luaL_checkudata(L, 1, "INIConfig");

    if (pIniUserData->pConfig) 
    {
        ini_free(pIniUserData->pConfig);
        pIniUserData->pConfig = NULL;
    }

    return 0;
}

CAPI int
LUA_INIGet(struct lua_State* L)
{
    lua_ini_t *pIniUserData = (lua_ini_t *)luaL_checkudata(L, 1, "INIConfig");
    const char *sSection    = luaL_checkstring(L, 2);
    const char *sKey        = luaL_checkstring(L, 3);
    const char *sValue      = ini_get(pIniUserData->pConfig, sSection, sKey);

    if (sValue) 
    {
        lua_pushstring(L, sValue);
    } 
    else 
    {
        lua_pushnil(L);
    }

    return 1;
}

