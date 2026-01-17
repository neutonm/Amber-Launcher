#ifndef __AMBER_LAUNCHER_COMMAND_ARCHIVE_H
#define __AMBER_LAUNCHER_COMMAND_ARCHIVE_H

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
 * @brief                   Extracts mod data packed in *.zip into game folder.
 * 
 * @param pSelf 
 * @param pArg 
 */
extern CAPI CBOOL
SCommand_Callback_Archive(const struct SCommand* pSelf, const SVar* pArgs, const unsigned int dNumArgs);

extern CAPI int
LUA_ArchiveExtract(struct lua_State* L);

__END_C

#endif
