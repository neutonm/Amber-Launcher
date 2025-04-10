#ifndef __AMBER_LAUNCHER_COMMAND_ARCHIVE_H
#define __AMBER_LAUNCHER_COMMAND_ARCHIVE_H

#include <core/common.h>

struct SCommand;
struct SCommandArg;
struct lua_State;

/**
 * @relatedalso             Commands
 * @brief                   Extracts mod data packed in *.zip into game folder.
 * 
 * @param pSelf 
 * @param pArg 
 */
extern CAPI CBOOL
SCommand_Callback_Archive(const struct SCommand* pSelf, const struct SCommandArg* pArg);

extern CAPI int
LUA_ArchiveExtract(struct lua_State* L);

#endif
