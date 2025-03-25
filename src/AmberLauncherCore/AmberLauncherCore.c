#include "core/common.h"
#include "lua.h"
#include <AmberLauncherCore.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <core/appcore.h>
#include <core/command.h>
#include <core/luastate.h>
#include <core/opsys.h>

#include <commands/archive.h>
#include <commands/config.h>
#include <commands/regedit.h>
#include <commands/music.h>

#include <lauxlib.h>

static const char* STR_AL_GLOBAL    = "AL";
static const char* STR_AL_APPCORE   = "AL.AppCore";

static void 
_SCommand_Callback_Null(const SCommand* pSelf, const SCommandArg* pArg)
{
    UNUSED(pSelf);
    UNUSED(pArg);

    printf("Null command call\n");
}

static const CommandFunc 
SVector_DefaultCommandCallbacks[] =
{
    _SCommand_Callback_Null,
    /* SCommand_Callback_Archive, */
    /* SCommand_Callback_ConvertMusic, */
    /* SCommand_Callback_RegEdit, */
    /* SCommand_Callback_TweakConfig */
};

static const char* 
SVector_DefaultCommandCallbackNames[] =
{
    "Null",
    /* "ExtractArchives", */
    /* "ConvertMusic", */
    /* "RegistryTweaks", */
    /* "ConfigTweaks" */
};

static const size_t SVector_DefaultCommandCallbacks_Size = 
    sizeof(SVector_DefaultCommandCallbacks) / sizeof(SVector_DefaultCommandCallbacks[0]);

#define SVECTOR_MAX_COMMANDS 32
static SVector tConfigureCommandList;

static int 
_predicate(const void* pElement, const void* pCtx)
{
    SCommand* pObject = (SCommand*)pElement;
    const char* sName = (const char*)pCtx;
    const int dResult = strcmp(pObject->sName, sName);
    return dResult == 0;
}

static void 
_SCommand_Callback_LuaCall(const SCommand* pSelf, const SCommandArg* pArg)
{
    const int dLuaRef = pSelf->dLuaRef;
    struct lua_State *L = NULL;

    if (IS_VALID(pArg) && pArg->eType == CTYPE_VOID)
    {
        L = (lua_State*)pArg->uData.pValue;
        assert(IS_VALID(L));
    }

    /* Push the Lua function onto the stack using the reference */
    lua_rawgeti(L, LUA_REGISTRYINDEX, dLuaRef);

    if (lua_pcall(L, 0, 0, 0) != LUA_OK) 
    {
        /* Handle error */
        fprintf
        (
            stderr, 
            "Error calling Lua command '%s': %s\n", pSelf->sName, 
            lua_tostring(L, -1)
        );
        lua_pop(L, 1);
    }
}

static void
_SCommand_AddToList(const char* sName, CommandFunc cbCmdFunction, void* pOwner, int dLuaRef, int dPriority)
{
    long dSVecFoundIndex;

    SCommand tCommand;
    tCommand.sName          = sName;
    tCommand.dFlags         = dLuaRef > 0 ? SCOMMAND_FLAG_LUA : SCOMMAND_FLAG_NULL;
    tCommand.dLuaRef        = dLuaRef;
    tCommand.dPriority      = dPriority;
    tCommand.dNumArgs       = 0;
    tCommand.pOwner         = pOwner;
    tCommand.cbExecuteFunc  = cbCmdFunction;

    /* Overwrite existing */
    dSVecFoundIndex = SVector_FindByPredicate(&tConfigureCommandList, _predicate, sName);
    if (dSVecFoundIndex != -1)
    {
        SCommand* pObj  = (SCommand*)SVector_Get(&tConfigureCommandList, (size_t)dSVecFoundIndex);
        pObj->dFlags    = tCommand.dFlags;
        pObj->dLuaRef   = tCommand.dLuaRef;
        pObj->dPriority = tCommand.dPriority;

        if (SCommand_IsFlagSet(pObj, SCOMMAND_FLAG_LUA))
        {
            pObj->cbExecuteFunc = _SCommand_Callback_LuaCall;
        }

        return;
    }

    /* Add new element */
    SVector_PushBack(&tConfigureCommandList, &tCommand);
}

static int
_LUA_CommandAdd(struct lua_State* L)
{
    int dPriority           = 0;
    int dLuaFuncRef         = 0;
    void* pAppCore          = NULL; /*!< AppCore */
    const char* sCmdName    = luaL_checkstring(L, 1);

    luaL_checktype(L, 2, LUA_TFUNCTION);
    dPriority = (int)luaL_optinteger(L, 3, 0);

    lua_pushvalue(L, 2);
    dLuaFuncRef = luaL_ref(L, LUA_REGISTRYINDEX);

    lua_getfield(L, LUA_REGISTRYINDEX, STR_AL_APPCORE);
    pAppCore = lua_touserdata(L, -1);
    lua_pop(L, 1);

    _SCommand_AddToList(sCmdName, _SCommand_Callback_LuaCall, pAppCore, dLuaFuncRef, dPriority);

    return 0;
}

static int 
_LUA_GetTableOfCommands(struct lua_State* L)
{
    int dLuaIndex = 1;

    lua_newtable(L);

    SVector_ForEach(&tConfigureCommandList)
    {
        SVector_InitIterator(SCommand, &tConfigureCommandList);

        lua_newtable(L);

        lua_pushstring(L, SVECTOR_ITERATOR->sName);
        lua_setfield(L, -2, "name");

        lua_pushinteger(L, SVECTOR_ITERATOR->dPriority);
        lua_setfield(L, -2, "priority");

        lua_rawseti(L, -2, dLuaIndex);
        dLuaIndex++;
    }

    return 1;
}

static int 
_LUA_CommandCall(struct lua_State* L)
{
    long dSVecFoundIndex     = 0;
    const char      *sKey    = luaL_checkstring(L, 1);

    dSVecFoundIndex = SVector_FindByPredicate(&tConfigureCommandList, _predicate, sKey);
    if (dSVecFoundIndex != -1)
    {
        SCommand* pObj = (SCommand*)SVector_Get(&tConfigureCommandList, (size_t)dSVecFoundIndex);
        SCommandArg pArg = SCommandArg_MakeVoid(L);
        pObj->cbExecuteFunc(pObj, &pArg);
    }
    else
    {
        return luaL_error(L, "LUA_CallCommand failed to find element %s", sKey);
    }

    return 0;
}

static const 
luaL_Reg AL[] = 
{
    {"CommandAdd",                  _LUA_CommandAdd             },
    {"CommandCall",                 _LUA_CommandCall            },
    {"GetTableOfCommands",          _LUA_GetTableOfCommands     },
    {"SetRegistryKey",              LUA_SetRegistryKey          },
    {"GetRegistryKey",              LUA_GetRegistryKey          },
    {"ConvertMP3ToWAV",             LUA_ConvertMP3ToWAV         },
    {"ArchiveExtract",              LUA_ArchiveExtract          },
    {"INILoad",                     LUA_INILoad                 },
    {"INIClose",                    LUA_INIClose                },
    {"INIGet",                      LUA_INIGet                  },
    {NULL, NULL}
};

CAPI void
AmberLauncher_Start(AppCore* pAppCore)
{
    size_t i = 0;

    /* Register C functions to lua */
    luaL_newlib(pAppCore->pLuaState->pState, AL);
    lua_setglobal(pAppCore->pLuaState->pState, STR_AL_GLOBAL);

    /* Register pAppCore */
    lua_pushlightuserdata(pAppCore->pLuaState->pState, (void*)pAppCore);
    lua_setfield(pAppCore->pLuaState->pState, LUA_REGISTRYINDEX, STR_AL_APPCORE);

    LUA_REGISTER_INIConfig(pAppCore->pLuaState->pState);

    /* Command database */
    SVector_Init(&tConfigureCommandList, sizeof(SCommand));
    SVector_Reserve(&tConfigureCommandList, SVECTOR_MAX_COMMANDS);
    for (i = 0; i < SVector_DefaultCommandCallbacks_Size; i++)
    {
        _SCommand_AddToList(
            SVector_DefaultCommandCallbackNames[i],
            SVector_DefaultCommandCallbacks[i],
            pAppCore,
            0,
            (int)i);
    }
    
    /* Lua State */
    SLuaState_Init(pAppCore->pLuaState);
    SLuaState_CallEvent(pAppCore->pLuaState, ELuaFunctionEventTypeStrings[SLUA_EVENT_INIT]);
}

CAPI void
AmberLauncher_End(AppCore* pAppCore)
{
    /* Lua stuff */
    SLuaState_CallEvent(pAppCore->pLuaState, ELuaFunctionEventTypeStrings[SLUA_EVENT_DESTROY]);
    SLuaState_CallReferencedFunction(pAppCore->pLuaState, SLUA_FUNC_APPDESTROY);
    SVector_ForEach(&tConfigureCommandList)
    {
        SVector_InitIterator(SCommand, &tConfigureCommandList);
        if (SCommand_IsFlagSet(SVECTOR_ITERATOR, SCOMMAND_FLAG_LUA))
        {
            luaL_unref(pAppCore->pLuaState->pState, LUA_REGISTRYINDEX, SVECTOR_ITERATOR->dLuaRef);
        }
    }
    

    /* Other shit */
    SVector_Cleanup(&tConfigureCommandList);
}

CAPI void
AmberLauncher_Test(AppCore* pAppCore)
{
    UNUSED(pAppCore);
    /* if (pAppCore->pText)
    {
        textview_printf(pAppCore->pText, "Library call\n");
    } */

    /* SCommand_Callback_ConvertMusic(NULL,NULL); */
}

CAPI void
AmberLauncher_ConfigureStart(AppCore* pAppCore)
{
    UNUSED(pAppCore);
    /* if (pAppCore->pText)
    {
        textview_printf(pAppCore->pText, "Configuration start...\n");
    } */

    SLuaState_CallEvent(
        pAppCore->pLuaState, 
        ELuaFunctionEventTypeStrings[SLUA_EVENT_CONFIGURE_BEFORE]
    );

    /* AmberLauncher_Test(pAppCore); */

    SLuaState_CallReferencedFunction(pAppCore->pLuaState, SLUA_FUNC_APPCONFIGURE);

    SLuaState_CallEvent(
        pAppCore->pLuaState, 
        ELuaFunctionEventTypeStrings[SLUA_EVENT_CONFIGURE_AFTER]
    );

    fflush(stderr);
}

CAPI void
AmberLauncher_Play(AppCore* pAppCore)
{
    SLuaState_CallEvent(
        pAppCore->pLuaState, 
        ELuaFunctionEventTypeStrings[SLUA_EVENT_PLAY]
    );
    AmberLauncher_ProcessLaunch("mm7.exe", 0, NULL, CTRUE);
    UNUSED(pAppCore);
}

