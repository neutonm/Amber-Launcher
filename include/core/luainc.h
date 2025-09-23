#ifndef SLUAINCLUDE_H_
#define SLUAINCLUDE_H_

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

/******************************************************************************
 * HEADER FUNCTION DECLARATIONS
 ******************************************************************************/
#if LUA_VERSION_NUM == 501
extern void
luaL_setfuncs(lua_State *L, const luaL_Reg *l, int nup);

extern void
luaL_traceback(lua_State *L, lua_State *L1, const char *msg, int level);
#endif

/******************************************************************************
 * MACROS
 ******************************************************************************/
#if LUA_VERSION_NUM == 501
    #define luaL_newlib(L, l)            \
        (lua_newtable((L)),              \
        luaL_setfuncs((L), (l), 0))

    #ifndef LUA_OK /* introduced in 5.2 */
        #define LUA_OK 0
    #endif
#endif

#endif
