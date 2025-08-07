#include "core/common.h"
#include <core/luastate.h>

#include <core/luacommon.h>
#include <core/command.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#elif defined(__unix__)
#include <dirent.h>
#include <sys/stat.h>
#endif

/******************************************************************************
 * MACROS
 ******************************************************************************/

/* Lua 5.1 (or less) compactibility define */
#ifndef LUA_OK
#define LUA_OK 0
#endif

#define MAX_TABLE_DEPTH  8

/******************************************************************************
 * STATIC FUNCTION DECLARATION
 ******************************************************************************/
static void
_LoadAllLuaScripts(lua_State* L, const char* folder_path);

static CBOOL
_LuaObjectToSVar(
    lua_State *L,
    SVar *pOut,
    int dIdx,
    int dDepth);

static CBOOL
_LuaTableToSVar(
    lua_State *L,
    int dIdx,
    SVar *pOut,
    int depth);

/******************************************************************************
 * HEADER FUNCTION DEFINITIONS
 ******************************************************************************/

CAPI SLuaState* 
SLuaState_new(void)
{
    SLuaState* pLuaState = (SLuaState*)malloc(sizeof(SLuaState));
    if (!IS_VALID(pLuaState))
    {
        fprintf
        (
            stderr,
            "SLuaState_new() -> Failed to allocate memory."
        );
        return NULL;
    }

    /* Initialize variables */
    pLuaState->pState               = luaL_newstate();
    pLuaState->sScriptNameFolder    = "Scripts/Launcher";
    pLuaState->sScriptNameConst     = "_const";
    pLuaState->sScriptNameMain      = "_main";
    luaL_openlibs(pLuaState->pState);

    SVector_Init(
        &pLuaState->tRefFunctions, sizeof(SLuaFunctionRef));
        
    SVector_Reserve(&pLuaState->tRefFunctions, SLUA_FUNC_MAX);

    return pLuaState;
}

CAPI CBOOL 
SLuaState_delete(SLuaState** pLuaState)
{
    if (!IS_VALID(pLuaState) || !IS_VALID(*pLuaState))
    {
        fprintf
        (
            stderr,
            "SLuaState_delete(SLuaState** LuaState) -> \
            received \"LuaState\" as NULL."
        );
        return CFALSE;
    }
    
    lua_close((*pLuaState)->pState);
    (*pLuaState)->pState = NULL;
    
    SVector_Cleanup(&(*pLuaState)->tRefFunctions);
    
    free(*pLuaState);
    *pLuaState = NULL;

    return CTRUE;
}

CAPI CBOOL
SLuaState_Init(SLuaState* pLuaState)
{
    int i;

    if (!IS_VALID(pLuaState) || !IS_VALID(pLuaState->pState))
    {
        fprintf
        (   
            stderr, 
            "SLuaState_Init(SLuaState* pLuaState)-> \
            received \"pLuaState\" as NULL or LuaState->pState is NULL."
        );
        return CFALSE;
    }

    if (!SLuaState_LoadScript(pLuaState, pLuaState->sScriptNameConst))
    {
        fprintf
        (
            stderr, 
            "SLuaState_Init"
            "(SLuaState* pLuaState) -> "
            "Failed to load \"%s\" lua script.",
            pLuaState->sScriptNameConst
        );

        return CFALSE;
    }

    if (!SLuaState_LoadScript(pLuaState, pLuaState->sScriptNameMain))
    {
        fprintf
        (
            stderr, 
            "SLuaState_Init"
            "(SLuaState* pLuaState) -> "
            "Failed to load \"%s\" lua script.",
            pLuaState->sScriptNameMain
        );

        return CFALSE;
    }

    /* Register global functions */
    for(i = 0; i < SLUA_FUNC_MAX; i++)
    {
        SLuaState_RegisterFunctionReference(
            pLuaState, 
            (ELuaFunctionRefType)i, 
            ELuaFunctionRefTypeStrings[i]
        );
    }
    
    /* SLuaState_PushGlobalVariable(pLuaState, sLuaBackendName, "true", SLUA_TYPE_BOOL); */
    SLuaState_CallReferencedFunction(pLuaState, SLUA_FUNC_APPINIT);

    _LoadAllLuaScripts(pLuaState->pState, pLuaState->sScriptNameFolder);

    SLuaState_CallReferencedFunction(pLuaState, SLUA_FUNC_POST_APPINIT);

    return CTRUE;
}

CAPI CBOOL 
SLuaState_RegisterFunctionReference(
    SLuaState* pLuaState, 
    ELuaFunctionRefType eRefType, 
    const char* sLuaFunctionName)
{
    SLuaFunctionRef tLuaFuncRef;
    UNUSED(eRefType);
    /** @todo Luastate: Proper error handle */
    lua_getglobal(pLuaState->pState, sLuaFunctionName);
    if (!lua_isfunction(pLuaState->pState, -1)) 
    {
        fprintf
        (   
            stderr, 
            "SLuaState_RegisterFunctionReference"
            "(SLuaState* pLuaState, ELuaFunctionRefType RefType,"
            "const char* LuaFunctionName) -> "
            "Lua function not found: %s",
            sLuaFunctionName
        );

        return CFALSE;
    } 
    
    tLuaFuncRef.sName   = sLuaFunctionName;
    tLuaFuncRef.dRef    = luaL_ref(pLuaState->pState, LUA_REGISTRYINDEX);;

    SVector_PushBack(&pLuaState->tRefFunctions, &tLuaFuncRef);

    return CTRUE;
}

CAPI CBOOL 
SLuaState_GetGlobalVariable(
    const SLuaState* pLuaState, 
    const char* sVariableName, 
    SLuaVar* pVarOut) 
{
    lua_State* L = pLuaState->pState;

    if (!IS_VALID(pLuaState) || !IS_VALID(pLuaState->pState) || !IS_VALID(sVariableName))
    {
        fprintf
        (   
            stderr, 
            "SLuaState_GetGlobalVariable"
            "(SLuaState* pLuaState, const char* sVariableName, SLuaVar* pVarOut) -> "
            "received \"LuaState\" as NULL."
        );
        return CFALSE;
    }

    lua_getglobal(L, sVariableName);

    switch (lua_type(L, -1)) 
    {
        case LUA_TNIL:
            pVarOut->type           = SLUA_TYPE_NIL;
            break;
        case LUA_TNUMBER:
            pVarOut->type           = SLUA_TYPE_NUMBER;
            pVarOut->data.num       = lua_tonumber(L, -1);
            break;
        case LUA_TBOOLEAN:
            pVarOut->type           = SLUA_TYPE_BOOL;
            pVarOut->data.boolean   = lua_toboolean(L, -1);
            break;
        case LUA_TSTRING:
            pVarOut->type           = SLUA_TYPE_STRING;
            pVarOut->data.str       = lua_tostring(L, -1);
            break;
        case LUA_TFUNCTION:
            pVarOut->type           = SLUA_TYPE_FUNC;
            /* @todo implement lua_CFunction to be stored inside SLuaVar */
            /* pVarOut->userdata       = (void*)lua_tocfunction(L, -1); */
            break;
        default:
            fprintf
            (   
                stderr, 
                "SLuaState_GetGlobalVariable"
                "(SLuaState* pLuaState, "
                "const char* sVariableName, "
                "SLuaVar* pVarOut) -> "
                "Unsupported Lua type for variable \"%s\"",
                sVariableName
            );
            lua_pop(L, 1);
            return CFALSE;
    }

    lua_pop(L, 1);
    return CTRUE;
}

CAPI CBOOL 
SLuaVar_IsOfType(const SLuaVar* pVar, ELuaVarType eType)
{
    /** remove this func + there are already lua object types defined */
    if (!IS_VALID(pVar))
    {
        return CFALSE;
    }

    return pVar->type == eType;
}

CAPI CBOOL 
SLuaState_ExecuteCode(SLuaState* pLuaState, const char *sCode)
{
    /* @todo: refactor Execute Lua Code function */
    if (!IS_VALID(pLuaState) || !IS_VALID(pLuaState->pState))
    {
        fprintf
        (   
            stderr, 
            "SLuaState_ExecuteCode(SLuaState* pLuaState, const char *sCode)-> \
            received \"LuaState\" as NULL or LuaState->pState is NULL."
        );
        return CFALSE;
    }

    if (luaL_dostring(pLuaState->pState, sCode)) 
    {
        fprintf(stderr, "Error: %s\n", lua_tostring(pLuaState->pState, -1));
        lua_pop(pLuaState->pState, 1);
        
        return CFALSE;
    } 
    else 
    {
        
        /* Print the result from the Lua stack */
        if (lua_gettop(pLuaState->pState) > 0 && lua_isstring(pLuaState->pState, -1)) 
        {
            /* printf("%s\n", lua_tostring(pLuaState->pState, -1)); */
            lua_pop(pLuaState->pState, 1);
        }
    }

    return CTRUE;
}

CAPI CBOOL 
SLuaState_LoadScript(SLuaState* pLuaState, const char* sScriptPath) 
{
    int iLuaRetValue = 0;
    char sBufferPath[256];
    char sSlashSymbol;

    if (!IS_VALID(pLuaState) || !IS_VALID(sScriptPath))
    {
        fprintf
        (   
            stderr, 
            "SLuaState_LoadScript"
            "(SLuaState* pLuaState, const char* sScriptPath) -> "
            "received \"pLuaState\" as NULL or \"sScriptPath\" is empty."
        );
        
        return CFALSE;
    }

    /* Format path. @todo Write or use cross-platform solution to file paths */
#ifdef _WIN32
    sSlashSymbol = '\\';
#else
    sSlashSymbol = '/';
#endif
    sprintf(sBufferPath, "%s%c%s.lua", 
        pLuaState->sScriptNameFolder, 
        pLuaState->sScriptNameFolder ? sSlashSymbol : 0,
        sScriptPath);

    /* Load script */
    iLuaRetValue = luaL_loadfile(pLuaState->pState, sBufferPath);
    if (iLuaRetValue != LUA_OK) 
    {
        const char* sLuaMessage = lua_tostring(pLuaState->pState, -1);
        
        fprintf
        (
            stderr, 
            "SLuaState_LoadScript"
            "(SLuaState* LuaState, const char* scriptPath) -> "
            "Error loading Lua script: %s",
            (IS_VALID(sLuaMessage) ? sLuaMessage : "<Unknown>")
        );

        lua_pop(pLuaState->pState, 1);
        return CFALSE;
    }

    iLuaRetValue = lua_pcall(pLuaState->pState, 0, LUA_MULTRET, 0);
    if (iLuaRetValue != LUA_OK) 
    {
        const char* sLuaMessage = lua_tostring(pLuaState->pState, -1);

        fprintf
        (   
            stderr, 
            "SLuaState_LoadScript"
            "(SLuaState* pLuaState, const char* sScriptPath) -> "
            "Error executing Lua script: %s",
            (IS_VALID(sLuaMessage) ? sLuaMessage : "<Unknown>")
        );

        lua_pop(pLuaState->pState, 1);
        return CFALSE;
    }

    return CTRUE;
}

CAPI CBOOL 
SLuaState_CallReferencedFunction(
    SLuaState* pLuaState, 
    ELuaFunctionRefType eRefType)
{
    /** @todo Luastate: Proper error handle */
    const SLuaFunctionRef* pLuaFuncRef;

    pLuaFuncRef = (SLuaFunctionRef*)SVector_Get(&pLuaState->tRefFunctions, eRefType);
    lua_rawgeti(
        pLuaState->pState, 
        LUA_REGISTRYINDEX, 
        pLuaFuncRef->dRef);

    if (lua_pcall(pLuaState->pState, 0, 0, 0) != LUA_OK) 
    {
        const char *sErrorMessage = lua_tostring(pLuaState->pState, -1);

        fprintf
        (   
            stderr, 
            "SLuaState_CallReferencedFunction"
            "(SLuaState* LuaState, ELuaFunctionRefType RefType) -> "
            "Error calling function: %s\n",
            sErrorMessage
        );

        lua_pop(pLuaState->pState, 1);
        return CFALSE;
    }

    return CTRUE;
}

CAPI void
SLuaState_CallEvent(SLuaState* pLuaState, const char* sEventName)
{
    SLuaState_CallEventArgs(pLuaState, sEventName, NULL, 0);
}

CAPI void
SLuaState_CallEventArgs(SLuaState* pLuaState, const char* sEventName, const SVar* pArgs, size_t dNumArgs)
{
    size_t i;
    lua_State *L = pLuaState->pState;

    /* Call it as function events.<functionName>() */
    lua_getglobal(L, "events");
    if (!lua_istable(L, -1))
    {
        lua_pop(L, 1);
        return;
    }

    lua_getfield(L, -1, sEventName);
    if (!lua_isfunction(L, -1))
    {
        lua_pop(L, 2);
        return;
    }

    /* Process and push arguments */
    for (i = 0; i < dNumArgs; i++)
    {
        SLuaState_PushVariable(pLuaState, &pArgs[i]);
    }

    /* Execute function */
    if (lua_pcall(L, (int)dNumArgs, 0, 0) != LUA_OK)
    {
        fprintf
        (
            stderr,
            "Error calling event handler '%s': %s\n",
            sEventName, lua_tostring(L, -1)
        );
        lua_pop(L, 1);
    }

    lua_pop(L, 1);
}

CAPI void
SLuaState_PushVariable(SLuaState* pLuaState, const SVar *pVar)
{
    lua_State *L = pLuaState->pState;

    switch (pVar->eType)
    {
        case CTYPE_BOOL:
            lua_pushboolean(L, pVar->uData._bool);
            break;
        case CTYPE_CHAR:
            lua_pushinteger(L, pVar->uData._char);
            break;
        case CTYPE_UNSIGNED_CHAR:
            lua_pushinteger(L, pVar->uData._uchar);
            break;
        case CTYPE_CONST_CHAR:
            lua_pushstring(L, pVar->uData._constchar);
            break;
        case CTYPE_SHORT:
            lua_pushinteger(L, pVar->uData._short);
            break;
        case CTYPE_UNSIGNED_SHORT:
            lua_pushinteger(L, pVar->uData._ushort);
            break;
        case CTYPE_INT:
            lua_pushinteger(L, pVar->uData._int);
            break;
        case CTYPE_UNSIGNED_INT:
            lua_pushinteger(L, pVar->uData._uint);
            break;
        case CTYPE_LONG:
            lua_pushinteger(L, pVar->uData._long);
            break;
        case CTYPE_UNSIGNED_LONG:
            lua_pushinteger(L, pVar->uData._ulong);
            break;
        case CTYPE_LONG_LONG:
            lua_pushinteger(L, pVar->uData._longlong);
            break;
        case CTYPE_UNSIGNED_LONG_LONG:
            lua_pushinteger(L, pVar->uData._ulonglong);
            break;
        case CTYPE_FLOAT:
            lua_pushnumber(L, (lua_Number)pVar->uData._float);
            break;
        case CTYPE_DOUBLE:
            lua_pushnumber(L, pVar->uData._double);
            break;
        case CTYPE_LONG_DOUBLE:
            lua_pushnumber(L, (lua_Number)pVar->uData._longdouble);
            break;
        case CTYPE_VOID:
            lua_pushlightuserdata(L, pVar->uData._void);
            break;
        case CTYPE_LUAREF:
            lua_rawgeti(L, LUA_REGISTRYINDEX, pVar->uData._int);
            break;
        case CTYPE_LUATABLE:
            {
                size_t j;
                SVarTable *t = pVar->uData._void;
                lua_newtable(L); /* push empty */
                for (j = 0; j < t->dCount; ++j)
                {
                    SLuaState_PushVariable(pLuaState, &t->pEntries[j].tKey);
                    SLuaState_PushVariable(pLuaState, &t->pEntries[j].tValue);
                    lua_settable(L, -3); /* table[key]=value */
                }
            }
            break;
        case CTYPE_NULL:
        default:
            lua_pushnil(L);
            break;
    }
}

CAPI void 
SLuaState_ClearGlobalTable(SLuaState* pLuaState)
{
    if (!IS_VALID(pLuaState) || !IS_VALID(pLuaState->pState))
    {
        fprintf
        (
            stderr,
            "SLuaState_ClearGlobalTable(SLuaState** LuaState)-> \
            received \"LuaState\" as NULL or LuaState->pState is NULL."
        );
        return;
    }

    lua_pushnil(pLuaState->pState);
    lua_setglobal(pLuaState->pState, "_G");
}

CAPI CBOOL
SLuaState_LuaObjectToSVar(
    SLuaState* pLuaState,
    SVar* pOut,
    int dIdx,
    int dDepth
)
{
    if (!IS_VALID(pLuaState) || !IS_VALID(pLuaState->pState))
    {
        fprintf
        (
            stderr,
            "SLuaState_LuaObjectToSVar(SLuaState* pLuaState, SVar* pOut, int dIdx, int dDepth)-> \
            received \"LuaState\" as NULL or LuaState->pState is NULL."
        );
        return CFALSE;
    }

    return _LuaObjectToSVar(pLuaState->pState, pOut, dIdx, dDepth);
}

/******************************************************************************
 * HEADER CALLBACK DEFINITIONS
 ******************************************************************************/

CAPI void
SCommand_ExecuteLuaBasedCommand(
    const SCommand* pSelf, 
    const SCommandArg* pArg
)
{
    lua_State* L = (lua_State*)pSelf->pOwner;
    lua_rawgeti(L, LUA_REGISTRYINDEX, pSelf->dLuaRef);
    
    if (!lua_isfunction(L, -1)) 
    {
        lua_pop(L, 1);
        fprintf(stderr, "Error: Command '%s' does not have a valid Lua function.\n", pSelf->sName);
        return;
    }
    
    if (pSelf->dNumArgs >= 1 && pArg && pArg->eType == CTYPE_CHAR && pArg->uData.sValue) 
    {
        lua_pushstring(L, pArg->uData.sValue);
    }
    
    if (lua_pcall(L, pSelf->dNumArgs, 0, 0) != LUA_OK) 
    {
        const char* error_msg = lua_tostring(L, -1);
        fprintf(stderr, "Error executing Lua command '%s': %s\n", pSelf->sName, error_msg);
        lua_pop(L, 1);
    }
}

/******************************************************************************
 * STATIC FUNCTION DEFINITIONS
 ******************************************************************************/

static void
_LoadAllLuaScripts(lua_State* L, const char* folder_path)
{
#ifdef _WIN32

    WIN32_FIND_DATA find_data;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    char search_path[MAX_PATH];
    char script_path[MAX_PATH];

    snprintf(search_path, sizeof(search_path), "%s\\*.lua", folder_path);

    hFind = FindFirstFile(search_path, &find_data);

    if (hFind == INVALID_HANDLE_VALUE) 
    {
        fprintf(stderr, "Error opening directory: %s\n", folder_path);
        return;
    }

    do 
    {
        /* Exclude scripts that start with '_' */
        if (find_data.cFileName[0] == '_') {
            continue; 
        }

        /* Build the full path to the script */
        snprintf(script_path, sizeof(script_path), "%s\\%s", folder_path, find_data.cFileName);

        if (luaL_dofile(L, script_path) != LUA_OK) 
        {
            fprintf(stderr, "Error loading %s: %s\n", script_path, lua_tostring(L, -1));
            lua_pop(L, 1);
        }

    } while (FindNextFile(hFind, &find_data) != 0);

    FindClose(hFind);

#else

    struct dirent* entry;
    char script_path[512];
    size_t name_len;

    DIR* dir = opendir(folder_path);
    if (!dir) 
    {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dir)) != NULL) 
    {
        struct stat path_stat;
        if (entry->d_name[0] == '.') 
        {
            continue;
        }

        /* Exclude scripts that start with '_' */
        if (entry->d_name[0] == '_') 
        {
            continue;
        }

        name_len = strlen(entry->d_name);
        if (name_len < 4 || strcmp(entry->d_name + name_len - 4, ".lua") != 0) 
        {
            continue; 
        }

        /* Build the full path to the file */
        snprintf(script_path, sizeof(script_path), "%s/%s", folder_path, entry->d_name);

        if (stat(script_path, &path_stat) != 0) 
        {
            perror("stat");
            continue;
        }

        if (!S_ISREG(path_stat.st_mode)) 
        {
            continue;
        }

        if (luaL_dofile(L, script_path) != LUA_OK) 
        {
            fprintf(stderr, "Error loading %s: %s\n", script_path, lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    }
    closedir(dir);
#endif
}

static CBOOL
_LuaObjectToSVar(
    lua_State *L,
    SVar *pOut,
    int dIdx,
    int dDepth)
{
    int dType = lua_type(L, dIdx);
    switch (dType)
    {
        case LUA_TNIL:
            SVAR_NULL(*pOut);
            break;
        case LUA_TBOOLEAN:
            SVAR_BOOL(*pOut,lua_toboolean(L, dIdx));
            break;
        case LUA_TNUMBER:
            SVAR_DOUBLE(*pOut,lua_tonumber(L, dIdx));
            break;
        case LUA_TSTRING:
            SVAR_CONSTCHAR(*pOut,lua_tostring(L, dIdx));
            break;
        case LUA_TTABLE:
            return _LuaTableToSVar(L, dIdx, pOut, dDepth);
        case LUA_TFUNCTION:
        case LUA_TUSERDATA:
        default:
            SVAR_LUAREF(*pOut, L, dIdx);
            pOut->dFlags = dType;
            break;
    }

    return CTRUE;
}

static CBOOL
_LuaTableToSVar(
    lua_State *L,
    int dIdx,
    SVar *pOut,
    int dDepth)
{
    SVarTable *pTable   = NULL;
    size_t dCapacity    = 8;
    size_t dCount       = 0;
    int dTableIdx       = 0;

    if (dDepth > MAX_TABLE_DEPTH)
    {
        return CFALSE;
    }

    pTable = (SVarTable*)calloc(1, sizeof(SVarTable));
    assert(IS_VALID(pTable));

    pTable->pEntries  = (SVarTableEntry*)calloc(dCapacity,sizeof(SVarTableEntry));
    if (!IS_VALID(pTable->pEntries))
    {
        free(pTable);
        return CFALSE;
    }

    lua_pushvalue(L, dIdx);
    dTableIdx = lua_gettop(L);

    /* keep a Lua ref so values remain valid if caller needs them */
    lua_pushvalue(L, dTableIdx);
    pTable->dLuaRef = luaL_ref(L, LUA_REGISTRYINDEX);

    lua_pushnil(L); /* first key */
    while (lua_next(L, dTableIdx) != 0)
    {
        if (dCount == dCapacity)
        {
            dCapacity <<= 1;
            pTable->pEntries = (SVarTableEntry*)realloc
            (
                pTable->pEntries,
                dCapacity * sizeof(SVarTableEntry)
            );
            assert(pTable->pEntries);
        }

        /* key at -2, value at -1 */
        _LuaObjectToSVar(L, &pTable->pEntries[dCount].tKey, -2, dDepth+1);
        _LuaObjectToSVar(L, &pTable->pEntries[dCount].tValue, -1, dDepth+1);
        ++dCount;

        lua_pop(L, 1);
    }

    lua_pop(L, 1);

    pTable->dCount    = dCount;
    pOut->eType       = CTYPE_LUATABLE;
    pOut->dSize       = sizeof(*pTable);
    pOut->uData._void = pTable;

    return CTRUE;
}

