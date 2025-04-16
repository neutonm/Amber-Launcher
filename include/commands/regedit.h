#ifndef __AMBER_LAUNCHER_COMMAND_REGEDIT_H
#define __AMBER_LAUNCHER_COMMAND_REGEDIT_H

#include <core/common.h>

struct SCommand;
struct SCommandArg;
struct lua_State;

/**
 * @relatedalso             Commands
 * @brief                   Tweaks WIN32 registry (affects linux wine as well)
 * 
 * @param pSelf 
 * @param pArg 
 */
extern CAPI CBOOL
SCommand_Callback_RegEdit(const struct SCommand* pSelf, const struct SCommandArg* pArgs, const unsigned int dNumArgs);

/**
 * @brief                   Lua Function to register keys
 * 
 * @param                   L 
 */
extern CAPI int 
LUA_SetRegistryKey(struct lua_State* L);

/**
 * @brief                   Lua Function to register keys
 * 
 * @param                   L 
 */
extern CAPI int 
LUA_GetRegistryKey(struct lua_State* L);

#endif
