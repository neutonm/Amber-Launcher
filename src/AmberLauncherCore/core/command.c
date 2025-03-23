#include <core/command.h>

#include <stdio.h>
#include <stdlib.h>

#include <lua.h>

/******************************************************************************
 * MACROS
 ******************************************************************************/

#define MAKE_AND_RETURN_SCOMMANDARG(TYPE, FIELD, VALUE) do { \
    SCommandArg tArg;                                       \
    tArg.eType = TYPE;                                      \
    tArg.dFlags = 0;                                        \
    tArg.uData.FIELD = VALUE;                               \
    return tArg;                                            \
} while(0)

/******************************************************************************
 * HEADER FUNCTION DEFINITIONS
 ******************************************************************************/

CAPI SCommand*
SCommand_new(void)
{
    SCommand* pCommand = (SCommand*)malloc(sizeof(SCommand));

    if (!IS_VALID(pCommand))
    {
        fprintf(
            stderr,   
            "SCommand_new() -> Failed to allocate memory."
        );

        return NULL;
    }

    pCommand->sName           = NULL;
    pCommand->pOwner          = NULL;
    pCommand->cbExecuteFunc   = NULL;
    pCommand->dFlags          = 0;
    pCommand->dNumArgs        = 0;
    pCommand->dLuaRef         = 0;
    pCommand->dPriority       = 0;

    return pCommand;
}

CAPI CBOOL
SCommand_delete(SCommand** pCommand)
{
    if (!IS_VALID(pCommand) || !IS_VALID(*pCommand))
    {
        fprintf(
            stderr, 
            "SCommand_delete() -> received \"pCommand\" as NULL.")
        ;

        return CFALSE;
    }

    free(*pCommand);
    *pCommand = NULL;

    return CTRUE;
}

CAPI CBOOL
SCommand_SetFlag(SCommand* pCommand, unsigned int dFlag)
{
    if (IS_VALID(pCommand)) 
    {
        pCommand->dFlags |= dFlag;
        return CTRUE;
    }
    return CFALSE;
}

CAPI CBOOL
SCommand_IsFlagSet(const SCommand* pCommand, unsigned int dFlag)
{
    return (pCommand && (pCommand->dFlags & dFlag)) ? CTRUE : CFALSE;
}

CAPI void
SCommand_ClearFlag(SCommand* pCommand, unsigned int dFlag)
{
    if (IS_VALID(pCommand)) 
    {
        pCommand->dFlags &= ~dFlag;
    }
}

CAPI SCommandArg 
SCommandArg_MakeNull(void) 
{
    SCommandArg tArg;
    tArg.uData.pValue  = NULL;
    tArg.dFlags        = 0;
    tArg.eType         = CTYPE_NULL;
    return tArg;
}

CAPI SCommandArg 
SCommandArg_MakeInt(int dVar) 
{
    MAKE_AND_RETURN_SCOMMANDARG(CTYPE_INT, dValue, dVar);
}

CAPI SCommandArg 
SCommandArg_MakeString(char* sVar) 
{
    /** @todo Ownership should belong to SCommandArg (const char*), use strdup or whatever, refactor */
    MAKE_AND_RETURN_SCOMMANDARG(CTYPE_CHAR, sValue, sVar);
}

CAPI SCommandArg 
SCommandArg_MakeVoid(void* pVar) 
{
    MAKE_AND_RETURN_SCOMMANDARG(CTYPE_VOID, pValue, pVar);
}

CAPI int
LUA_RegisterCommand(struct lua_State *L)
{
    UNUSED(L);
    return 0;
}
