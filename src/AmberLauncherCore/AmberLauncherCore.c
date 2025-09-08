#include "core/luacommon.h"
#include "version.h"
#include <AmberLauncherCore.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include <core/common.h>
#include <core/observer.h>
#include <core/vector.h>
#include <core/appcore.h>
#include <core/command.h>
#include <core/luastate.h>
#include <core/opsys.h>

#include <commands/archive.h>
#include <commands/config.h>
#include <commands/regedit.h>
#include <commands/music.h>

#include <ext/sha256.h>

#include <lua.h>
#include <lauxlib.h>

static const char* STR_AL_GLOBAL    = "AL";
static const char* STR_AL_APPCORE   = "AL.AppCore";

#define AL_SHA_BUF 65536U

static CBOOL
_SCommand_Callback_Null(const SCommand* pSelf, const SCommandArg* pArgs, const unsigned int dNumArgs)
{
    UNUSED(pSelf);
    UNUSED(pArgs);
    UNUSED(dNumArgs);

    printf("Null command call\n");

    return CTRUE;
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

static CBOOL
_SCommand_Callback_LuaCall(const SCommand* pSelf, const SCommandArg* pArgs, const unsigned int dNumArgs)
{
    const int dLuaRef   = pSelf->dLuaRef;
    struct lua_State *L = NULL;
    CBOOL bResult       = CFALSE;

    UNUSED(dNumArgs);

    if (IS_VALID(pArgs) && pArgs->eType == CTYPE_VOID)
    {
        L = (lua_State*)pArgs->uData.pValue;
        assert(IS_VALID(L));
    }

    /* Push the Lua function onto the stack using the reference */
    lua_rawgeti(L, LUA_REGISTRYINDEX, dLuaRef);

    if (lua_pcall(L, 0, 1, 0) != LUA_OK)
    {
        /* Handle error */
        fprintf
        (
            stderr, 
            "Error calling Lua command '%s': %s\n", pSelf->sName, 
            lua_tostring(L, -1)
        );
        lua_pop(L, 1);
        return CFALSE;
    }

    /* Handle 'bool' return from lua call */
    if (lua_isboolean(L, -1))
    {
        bResult = (CBOOL)lua_toboolean(L, -1);
    }
    lua_pop(L, 1);

    return bResult;
}

static CBOOL
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

        return CTRUE;
    }

    /* Add new element */
    SVector_PushBack(&tConfigureCommandList, &tCommand);

    return CTRUE;
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

static void
_DisposeRecursive(lua_State *L, SVar *pVar)
{
    size_t i;
    assert(IS_VALID(pVar));

    switch (pVar->eType)
    {
        case CTYPE_LUAREF:
            luaL_unref(L, LUA_REGISTRYINDEX, SVAR_GET_LUAREF(*pVar));
            break;

        case CTYPE_LUATABLE:
        {
            SVarTable *pTable = pVar->uData._void;
            assert(IS_VALID(pTable));

            for (i = 0; i < pTable->dCount; ++i)
            {
                _DisposeRecursive(L,&pTable->pEntries[i].tKey);
                _DisposeRecursive(L,&pTable->pEntries[i].tValue);
            }

            if (pTable->dLuaRef != LUA_REFNIL)
            {
                luaL_unref(L, LUA_REGISTRYINDEX, pTable->dLuaRef);
            }

            free(pTable->pEntries);
            free(pTable);
        }
        break;

        default:
            break;
    }
    pVar->eType = CTYPE_NULL;
}

static int
_LUA_UICall(lua_State* L)
{
    AppCore *pAppCore       = NULL;
    SVar    *pVarArgs       = NULL;
    uint32   dID            = 0;
    unsigned int i          = 0;
    unsigned int dNumArgs   = 0;
    SVarKeyBundle tRetVal;

    /* AppCore struct */
    lua_getfield(L, LUA_REGISTRYINDEX, STR_AL_APPCORE);
    pAppCore = lua_touserdata(L, -1);
    lua_pop(L, 1);

    if (pAppCore->cbUIEvent == NULL)
    {
        return luaL_error(L, "Appcore->cbUIEvent is NULL");
    }

    /* Ensure first argument as event name */
    dID = (uint32)luaL_checkinteger(L, 1);

    /* Acquire variadic args from lua */
    /** @todo this code repeats twice here, refactor */
    {
        unsigned int    dTop;
        unsigned int    dType;

        dTop            = lua_gettop(L);
        dType           = 0;
        pVarArgs        = (SVar*)calloc(dTop, sizeof(SVar));

        assert(pVarArgs);

        /* Loop through variadic args */
        for(i = 2; i <= dTop; i++)
        {
            dType = lua_type(L, i);
            printf("Arg[%d]: %s -> ", i, lua_typename(L, dType));

            if (lua_isnumber(L, i))
            {
                lua_Number fNum = lua_tonumber(L, i);
                printf("%.2f\n", fNum);
                SVAR_DOUBLE(pVarArgs[dNumArgs], fNum);
                dNumArgs++;
            }
            else if (lua_isboolean(L, i))
            {
                int bArg = lua_toboolean(L, i);
                printf("%s\n", bArg ? "true" : "false");
                SVAR_BOOL(pVarArgs[dNumArgs], bArg);
                dNumArgs++;
            }
            else if (lua_isstring(L, i))
            {
                const char* sArg = lua_tostring(L, i);
                printf("%s\n", sArg);
                SVAR_CONSTCHAR(pVarArgs[dNumArgs], sArg);
                dNumArgs++;
            }
            else
            {
                if (!SLuaState_LuaObjectToSVar(pAppCore->pLuaState, &pVarArgs[dNumArgs],i,  0)) {
                    printf("unsupported\n");
                }
                else
                {
                    printf("marshal[%d]: type %d\n", i, pVarArgs[dNumArgs].eType);
                    ++dNumArgs;
                }
            }
        }
    }

    /* Return Variable */
    tRetVal = pAppCore->cbUIEvent(pAppCore, dID, pVarArgs, dNumArgs);

    lua_createtable(L, 0, (int)tRetVal.dCount);
    for (i = 0; i < tRetVal.dCount; ++i)
    {
        SLuaState_PushVariable(pAppCore->pLuaState, &tRetVal.tKeys[i].tVar);
        lua_setfield(L, -2, tRetVal.tKeys[i].sKey); /* t[key] = value */
    }

    /* Cleanup */
    for(i = 0; i < dNumArgs; i++)
    {
        _DisposeRecursive(L, &pVarArgs[i]);
    }
    free(pVarArgs);

    return 1;
}

/* Call any event from lua */
static int
_LUA_EventCall(lua_State* L)
{
    const char     *sEventName;
    AppCore        *pAppCore = NULL;
    SVar           *pVarArgs = NULL;
    unsigned int    dNumArgs = 0;
    unsigned int    i;

    /* AppCore struct */
    lua_getfield(L, LUA_REGISTRYINDEX, STR_AL_APPCORE);
    pAppCore = lua_touserdata(L, -1);
    lua_pop(L, 1);

    /* Ensure first argument as event name */
    sEventName = luaL_checkstring(L, 1);

    /* Acquire variadic args from lua */
    {
        unsigned int    dTop;
        unsigned int    dType;

        dTop            = lua_gettop(L);
        dType           = 0;
        pVarArgs        = (SVar*)calloc(dTop, sizeof(SVar));

        assert(pVarArgs);

        /* Loop through variadic args */
        for(i = 2; i <= dTop; i++)
        {
            dType = lua_type(L, i);
            printf("Arg[%d]: %s -> ", i, lua_typename(L, dType));

            if (lua_isnumber(L, i))
            {
                lua_Number fNum = lua_tonumber(L, i);
                printf("%.2f\n", fNum);
                SVAR_DOUBLE(pVarArgs[dNumArgs], fNum);
                dNumArgs++;
            }
            else if (lua_isboolean(L, i))
            {
                int bArg = lua_toboolean(L, i);
                printf("%s\n", bArg ? "true" : "false");
                SVAR_BOOL(pVarArgs[dNumArgs], bArg);
                dNumArgs++;
            }
            else if (lua_isstring(L, i))
            {
                const char* sArg = lua_tostring(L, i);
                printf("%s\n", sArg);
                SVAR_CONSTCHAR(pVarArgs[dNumArgs], sArg);
                dNumArgs++;
            }
            else
            {
                switch (dType)
                {
                    case LUA_TTABLE:
                    case LUA_TFUNCTION:
                    case LUA_TUSERDATA:
                        {
                            const void* pLuaObj;

                            /* Get pointer to lua object (debug) */
                            lua_pushvalue(L, i);
                            pLuaObj = lua_topointer(L, -1);
                            lua_pop(L, 1);
                            printf("%p", pLuaObj);

                            /* Store reference */
                            SVAR_LUAREF(pVarArgs[dNumArgs], L, i);
                            dNumArgs++;
                        }
                        break;

                    default:
                        printf("???\n");
                        break;
                }
            }
        }
    }

    SSubject_Notify(pAppCore->pOnUserEventNotifier, 0, (void*)sEventName);
    SLuaState_CallEventArgs(pAppCore->pLuaState, sEventName, pVarArgs, dNumArgs);

    /* Cleanup */
    for(i = 0; i < dNumArgs; i++)
    {
        if (pVarArgs[i].eType == CTYPE_LUAREF)
        {
            luaL_unref(L, LUA_REGISTRYINDEX, SVAR_GET_LUAREF(pVarArgs[i]));
        }
    }
    free(pVarArgs);

    return 0;
}

static int
_LUA_CommandCall(struct lua_State* L)
{
    long dSVecFoundIndex     = 0;
    const char      *sKey    = luaL_checkstring(L, 1);
    CBOOL           bResult  = CTRUE;

    dSVecFoundIndex = SVector_FindByPredicate(&tConfigureCommandList, _predicate, sKey);
    if (dSVecFoundIndex != -1)
    {
        SCommand* pObj = (SCommand*)SVector_Get(&tConfigureCommandList, (size_t)dSVecFoundIndex);
        SCommandArg pArg = SCommandArg_MakeVoid(L);
        bResult = pObj->cbExecuteFunc(pObj, &pArg, 1);
        printf("Calling [%s]: %s\n", sKey, bResult ? "success!" : "fail.");
    }
    else
    {
        return luaL_error(L, "LUA_CallCommand failed to find element %s", sKey);
    }

    lua_pushboolean(L, (int)bResult);

    return 1;
}

static int
_LUA_SetLaunchCommand(lua_State* L)
{
    const char     *sNewLaunchCmd;
    AppCore        *pAppCore = NULL;

    /* AppCore struct */
    lua_getfield(L, LUA_REGISTRYINDEX, STR_AL_APPCORE);
    pAppCore = lua_touserdata(L, -1);
    lua_pop(L, 1);

    sNewLaunchCmd = luaL_checkstring(L, 1);
    AppCore_SetLaunchCommand(pAppCore, sNewLaunchCmd);

    return 0;
}

static int
_LUA_GetLaunchCommand(lua_State* L)
{
    AppCore        *pAppCore = NULL;

    /* AppCore struct */
    lua_getfield(L, LUA_REGISTRYINDEX, STR_AL_APPCORE);
    pAppCore = lua_touserdata(L, -1);
    lua_pop(L, 1);

    lua_pushstring(L, pAppCore->sLaunchCmd);

    return 1;
}

static const 
luaL_Reg AL[] = 
{
    {"CommandAdd",                  _LUA_CommandAdd             },
    {"CommandCall",                 _LUA_CommandCall            },
    {"GetTableOfCommands",          _LUA_GetTableOfCommands     },
    {"EventCall",                   _LUA_EventCall              },
    {"UICall",                      _LUA_UICall                 },
    {"SetLaunchCommand",            _LUA_SetLaunchCommand       },
    {"GetLaunchCommand",            _LUA_GetLaunchCommand       },
    {"SetRegistryKey",              LUA_SetRegistryKey          },
    {"GetRegistryKey",              LUA_GetRegistryKey          },
    {"ConvertMP3ToWAV",             LUA_ConvertMP3ToWAV         },
    {"ArchiveExtract",              LUA_ArchiveExtract          },
    {"INILoad",                     LUA_INILoad                 },
    {"INISave",                     LUA_INISave                 },
    {"INIClose",                    LUA_INIClose                },
    {"INIGet",                      LUA_INIGet                  },
    {"INISet",                      LUA_INISet                  },
    {NULL, NULL}
};

static void
_SHA256toHex(const unsigned char *sDigestIn, char *sHexOut)
{
    static const char tbl[] = "0123456789abcdef";
    size_t i;

    for (i = 0; i < SHA256_HASH_SIZE; ++i)
    {
        sHexOut[i * 2]     = tbl[sDigestIn[i] >> 4];
        sHexOut[i * 2 + 1] = tbl[sDigestIn[i] & 0x0F];
    }
    sHexOut[SHA256_HASH_SIZE * 2] = '\0';
}

CAPI void
AmberLauncher_Start(AppCore* pAppCore)
{
    size_t i = 0;

    /* Register C functions to lua */
    luaL_newlib(pAppCore->pLuaState->pState, AL);
    lua_setglobal(pAppCore->pLuaState->pState, STR_AL_GLOBAL);

    /* Debug const */
#if defined(NDEBUG)
    lua_pushboolean(pAppCore->pLuaState->pState, 0);
#else
    lua_pushboolean(pAppCore->pLuaState->pState, 1);
#endif
    lua_setglobal(pAppCore->pLuaState->pState, "_DEBUG");

    /* Build */
    lua_pushstring(pAppCore->pLuaState->pState, BUILD_NUMBER);
    lua_setglobal(pAppCore->pLuaState->pState, "_BUILD_NUMBER");

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
    SLuaState_CallReferencedFunction(pAppCore->pLuaState, SLUA_FUNC_APPDESTROY,NULL);
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
    SLuaState_CallReferencedFunction(pAppCore->pLuaState, SLUA_FUNC_POST_APPDESTROY,NULL);
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
    SVar tRetVal;

    SLuaState_CallEvent(
        pAppCore->pLuaState, 
        ELuaFunctionEventTypeStrings[SLUA_EVENT_CONFIGURE_BEFORE]
    );

    SLuaState_CallReferencedFunction(pAppCore->pLuaState, SLUA_FUNC_APPCONFIGURE,&tRetVal);

    if (SVAR_IS_NULL(tRetVal))
    {
        SLuaState_CallEvent(
            pAppCore->pLuaState,
            ELuaFunctionEventTypeStrings[SLUA_EVENT_CONFIGURE_AFTER]
        );
        SLuaState_CallReferencedFunction(
            pAppCore->pLuaState,
            SLUA_FUNC_POST_APPCONFIGURE,
            NULL
        );
    }
    else
    {
        SLuaState_CallEventArgs(
            pAppCore->pLuaState,
            ELuaFunctionEventTypeStrings[SLUA_EVENT_CONFIGURE_AFTER],
            &tRetVal,
            1
        );
        SLuaState_CallReferencedFunctionArgs(
            pAppCore->pLuaState,
            SLUA_FUNC_POST_APPCONFIGURE,
            NULL,
            &tRetVal,
            1
        );
    }

    fflush(stderr);
}

CAPI void
AmberLauncher_Play(AppCore* pAppCore)
{
    SLuaState_CallEvent(
        pAppCore->pLuaState, 
        ELuaFunctionEventTypeStrings[SLUA_EVENT_PLAY]
    );
    SLuaState_CallReferencedFunction(pAppCore->pLuaState, SLUA_FUNC_PLAY,NULL);
    AmberLauncher_ProcessLaunch(
        pAppCore->sLaunchCmd,
        0,
        NULL,
        CTRUE
    );
}

CAPI void
AmberLauncher_ExecuteLua(AppCore *pApp, const char *sCommand)
{
    SLuaState_ExecuteCode(pApp->pLuaState, sCommand);
}

CAPI SVar
AmberLauncher_GetGlobalVariable(AppCore *pApp, const char *sVarName)
{
    SLuaVar tLuaVar;
    SVar    tRetVar;

    SLuaState_GetGlobalVariable(pApp->pLuaState, sVarName, &tLuaVar);

    /* Convert to Svar */
    switch(tLuaVar.type)
    {
        case SLUA_TYPE_BOOL:
            SVAR_BOOL(tRetVar, tLuaVar.data.boolean);
            break;
        case SLUA_TYPE_NUMBER:
            SVAR_DOUBLE(tRetVar, tLuaVar.data.num);
            break;
        case SLUA_TYPE_STRING:
            SVAR_CONSTCHAR(tRetVar, tLuaVar.data.str);
            break;
        default:
            SVAR_NULL(tRetVar);
            break;
    }

    return tRetVar;
}

CAPI CBOOL
AmberLauncher_ProcessUISideButton(AppCore *pApp, unsigned int dButtonTag)
{
    SVar tVarButtonTag;
    SVar tVarRet;

    assert(IS_VALID(pApp));

    SVAR_UINT32(tVarButtonTag, dButtonTag);
    SLuaState_CallReferencedFunctionArgs(
        pApp->pLuaState,
        SLUA_FUNC_SIDEBUTTON,
        &tVarRet,
        &tVarButtonTag,
        1
    );

    if (SVAR_IS_NULL(tVarRet))
    {
        return CFALSE;
    }

    return SVAR_GET_BOOL(tVarRet);
}

CAPI CBOOL
AmberLauncher_ExecuteSVarLuaFunction(struct AppCore *pApp, const SVar *pVar)
{
    assert(IS_VALID(pApp));
    assert(IS_VALID(pVar));

    if (!SVAR_IS_LUAREF(*pVar))
    {
        return CFALSE;
    }

    if (!SLuaState_ExecuteLuaFunction(pApp->pLuaState, SVAR_GET_LUAREF(*pVar)))
    {
        return CFALSE;
    }

    return CTRUE;
}

char *
AmberLauncher_SHA256_HashFile(const char *sPath)
{
    FILE            *fp;
    unsigned char   sBuf[AL_SHA_BUF];
    size_t          dNread;
    SHA256_Context  tCtx;
    unsigned char   sDigest[SHA256_HASH_SIZE];
    char            *sHex;

    if (sPath == NULL)
    {
        errno = EINVAL;
        return NULL;
    }

    fp = fopen(sPath, "rb");
    if (fp == NULL)
    {
        return NULL;
    }

    sha256_initialize(&tCtx);

    while ((dNread = fread(sBuf, 1, AL_SHA_BUF, fp)) > 0) 
    {
        sha256_add_bytes(&tCtx, sBuf, dNread);
    }

    if (ferror(fp))
    {
        fclose(fp);
        return NULL;
    }
    fclose(fp);

    sha256_calculate(&tCtx, sDigest);

    sHex = (char *)malloc(SHA256_HASH_SIZE * 2 + 1);
    if (sHex == NULL)
        return NULL;

    _SHA256toHex(sDigest, sHex);
    return sHex;
}

