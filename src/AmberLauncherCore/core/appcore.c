#include <core/common.h>
#include <core/appcore.h>
#include <core/luastate.h>
#include <core/observer.h>

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static const char *sDefaultAppName = "mm7.exe";

/******************************************************************************
 * HEADER DEFINITIONS
 ******************************************************************************/

CAPI AppCore*
AppCore_create(void)
{
    AppCore* pAppCore = (AppCore*)malloc(sizeof(AppCore));
    if (!IS_VALID(pAppCore))
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
        pAppCore->argc                  = 0;
        pAppCore->argv                  = NULL;
        pAppCore->pLuaState             = SLuaState_new();
        pAppCore->pOnUserEventNotifier  = SSubject_new();
        pAppCore->sLaunchCmd            = NULL;

        AppCore_SetLaunchCommand(pAppCore, AppCore_GetDefaultLaunchCommand());

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
    SSubject_delete(&(*pAppCore)->pOnUserEventNotifier);

    free((*pAppCore)->sLaunchCmd);

    free(*pAppCore);
    *pAppCore = NULL;

    return CTRUE;
}

extern CAPI CBOOL
AppCore_SetLaunchCommand(AppCore* pAppCore, const char *sNewLaunchCmd)
{
    free(pAppCore->sLaunchCmd);

    pAppCore->sLaunchCmd = (char*)malloc(strlen(sNewLaunchCmd) + 1);
    if (!IS_VALID(pAppCore->sLaunchCmd))
    {
        return CFALSE;
    }

    strcpy(pAppCore->sLaunchCmd, sNewLaunchCmd);

    return CTRUE;
}

CAPI const char*
AppCore_GetDefaultLaunchCommand(void)
{
    return sDefaultAppName;
}
