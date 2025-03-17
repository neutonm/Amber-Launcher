#ifndef __AMBER_LAUNCHER_COMMAND_MUSIC_H
#define __AMBER_LAUNCHER_COMMAND_MUSIC_H

#include <core/common.h>

struct SCommand;
struct SCommandArg;
struct lua_State;

/**
 * @relatedalso             Commands
 * @brief                   Converts *.mp3 to *.wav in order to support music loop
 * 
 * @param pSelf 
 * @param pArg 
 */
extern CAPI void 
SCommand_Callback_ConvertMusic(const struct SCommand* pSelf, const struct SCommandArg* pArg);


extern CAPI int
LUA_ConvertMP3ToWAV(struct lua_State* L);

#endif
