#include "core/common.h"
#include <core/appcore.h>
#include <core/luastate.h>

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

/******************************************************************************
 * HEADER DEFINITIONS
 ******************************************************************************/

CAPI AppCore*
AppCore_create(void)
{
    AppCore* pAppCore = (AppCore*)malloc(sizeof(AppCore));
    if (pAppCore == NULL)
    {
        fprintf(
            stderr,
            "AppCore_create() -> Failed to allocate memory."
        );
    }
    return pAppCore;
}

CAPI CBOOL
AppCore_init(AppCore *pAppCore)
{
    if (IS_VALID(pAppCore))
    {
        pAppCore->argc      = 0;
        pAppCore->argv      = NULL;
        pAppCore->pLuaState = SLuaState_new();
        return CTRUE;
    }

    return CFALSE;
}

CAPI CBOOL
AppCore_free(AppCore** pAppCore)
{
    if (!IS_VALID(pAppCore) || !IS_VALID(*pAppCore))
    {
        fprintf
        (
            stderr,
            "AppCore_free() -> received \"pAppCore\" as NULL."
        );

        return CFALSE;
    }

    SLuaState_delete(&(*pAppCore)->pLuaState);

    free(*pAppCore);
    *pAppCore = NULL;

    return CTRUE;
}
