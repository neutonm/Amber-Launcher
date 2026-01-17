#ifndef __AMBER_LAUNCHER_COMMAND_MUSIC_H
#define __AMBER_LAUNCHER_COMMAND_MUSIC_H

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
 * @brief                   Converts *.mp3 to *.wav in order to support music loop
 * 
 * @param pSelf 
 * @param pArg 
 */
extern CAPI void 
SCommand_Callback_ConvertMusic(const struct SCommand* pSelf, const SVar* pArg, const unsigned int dNumArgs);

extern CAPI int
LUA_ConvertMP3ToWAV(struct lua_State* L);

__END_C

#endif
