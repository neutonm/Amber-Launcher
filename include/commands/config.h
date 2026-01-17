#ifndef __AMBER_LAUNCHER_COMMAND_INI_H
#define __AMBER_LAUNCHER_COMMAND_INI_H

#include <core/common.h>

/******************************************************************************
 * FORWARD DECLARATION
 ******************************************************************************/

struct SCommand;
struct lua_State;

/******************************************************************************
 * HEADER DECLARATIONS
 ******************************************************************************/

__EXTERN_C

/**
 * @relatedalso             Commands
 * @brief                   Tweaks mm7.ini
 * 
 * @param pSelf 
 * @param pArg 
 */
extern CAPI CBOOL
SCommand_Callback_TweakConfig(const struct SCommand* pSelf, const SVar* pArgs, const unsigned int dNumArgs);

extern CAPI int
LUA_REGISTER_INIConfig(struct lua_State* L);

extern CAPI int
LUA_INILoad(struct lua_State* L);

extern CAPI int
LUA_INISave(struct lua_State* L);

extern CAPI int
LUA_INIClose(struct lua_State* L);

extern CAPI int
LUA_INIGet(struct lua_State* L);

extern CAPI int
LUA_INISet(struct lua_State *L);

__END_C

#endif
