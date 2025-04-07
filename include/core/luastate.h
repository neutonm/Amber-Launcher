#ifndef SLUASTATE_H_
#define SLUASTATE_H_

#include <core/common.h>
#include <core/vector.h>
#include <core/luacommon.h>

__EXTERN_C

/******************************************************************************
 * FORWARD DECLARATIONS
 ******************************************************************************/

struct SCommand;
struct SCommandArg;
struct lua_State;

/******************************************************************************
 * STRUCTS
 ******************************************************************************/

/**
 * @brief   Holds Lua State and other related lua stuff
 * 
 */
typedef struct SLuaState
{
    SVECTOR_DECLARE(tRefFunctions, SLuaFunctionRef);
    struct lua_State* pState;
    const char* sScriptNameFolder; /** @todo refactor paths into enum+array */
    const char* sScriptNameConst;
    const char* sScriptNameMain;
    
} SLuaState;

/******************************************************************************
 * HEADER FUNCTION DECLARATIONS
 ******************************************************************************/

/**
 * @relatedalso     SLuaState
 * @brief           malloc and default variable initialization for SLuaState
 * 
 * @return          SLuaState* 
 */
extern CAPI struct SLuaState* 
SLuaState_new(void);

/**
 * @relatedalso     SLuaState
 * @brief           Cleanup function for SLuaState
 * 
 * @param           pLuaState 
 * @return          CBOOL 
 */
extern CAPI CBOOL 
SLuaState_delete(SLuaState** pLuaState);

/**
 * @relatedalso     SLuaState
 * @brief           Loads default scripts
 * 
 * @param           pLuaState 
 * @return          CBOOL 
 */
extern CAPI CBOOL
SLuaState_Init(SLuaState* pLuaState);

/**
 * @relatedalso     SLuaState
 * @brief           Registers function reference in lua state inside SLuaState's 
 *                  RefFunctions array;
 * 
 * @param           pLuaState 
 * @param           eRefType 
 * @param           sLuaFunctionName 
 * @return          CBOOL 
 */
extern CAPI CBOOL 
SLuaState_RegisterFunctionReference(
    SLuaState* pLuaState, 
    ELuaFunctionRefType eRefType, 
    const char* sLuaFunctionName
);

/**
 * @relatedalso     SLuaState
 * @brief           Pushes global variable of specified type into lua stack 
 * 
 * @param           pLuaState
 * @param           sVariableName
 * @param           sValue 
 * @param           eType 
 * @return          CBOOL 
 */
extern CAPI CBOOL
SLuaState_PushGlobalVariable(
    SLuaState* pLuaState,
    const char* sVariableName, 
    const char* sValue, 
    ELuaVarType eType);

/**
 * @relatedalso     pLuaState
 * @brief           Retrieves global variable from lua state
 * 
 * @param           pLuaState
 * @param           sVariableName
 * @param           pVarOut
 * @return          CBOOL
 */
extern CAPI CBOOL 
SLuaState_GetGlobalVariable(
    const SLuaState* pLuaState, 
    const char* sVariableName, 
    SLuaVar* pVarOut);

/**
 * @relatedalso     CLuaVar
 * @brief           Utility function: checks if CLuaVar is of specified type
 * 
 * @param           pVar
 * @param           eType 
 * @return          CBOOL 
 */
extern CAPI CBOOL 
SLuaVar_IsOfType(const SLuaVar* pVar, ELuaVarType eType);

/**
 * @relatedalso     SLuaState
 * @brief           Executes Lua code
 * 
 * @param           pLuaState 
 * @param           sCode 
 */
extern CAPI CBOOL 
SLuaState_ExecuteCode(SLuaState* pLuaState, const char *sCode);

/**
 * @relatedalso     SLuaState
 * @brief           Loads and Executes Lua Script
 * 
 * @param           pLuaState 
 * @param           sScriptPath 
 * @return          CBOOL 
 */
extern CAPI CBOOL 
SLuaState_LoadScript(SLuaState* pLuaState, const char* sScriptPath);

/**
 * @relatedalso     SLuaState
 * @brief           Calls registered referenced lua function state from SLuaState's 
 *                  RefFunctions array
 *
 * @param           pLuaState 
 * @param           eRefType 
 * @return          CBOOL 
 */
extern CAPI CBOOL 
SLuaState_CallReferencedFunction(
    SLuaState* pLuaState, 
    ELuaFunctionRefType eRefType
);

/**
 * @relatedalso     SLuaState
 * @brief           Calls app Lua event (function events.<sEventName>())
 *
 * @param           pLuaState
 * @param           sEventName
 */
extern CAPI void 
SLuaState_CallEvent(SLuaState* pLuaState, const char* sEventName);

/**
 * @relatedalso     SLuaState
 * @brief           Calls app Lua event with variadic arguments (function events.<sEventName>(...))
 *
 * @param           pLuaState
 * @param           sEventName
 */
extern CAPI void
SLuaState_CallEventArgs(SLuaState* pLuaState, const char* sEventName, const SVar* pArgs, size_t dNumArgs);

/**
 * @relatedalso     SLuaState
 * @brief           Pushes variable (SVar) into stack
 *
 * @param           pLuaState
 * @param           pVar
 */
extern CAPI void
SLuaState_PushVariable(SLuaState* pLuaState, const SVar *pVar);

/**
 * @relatedalso     SLuaState
 * @brief           Clears tables (Global Environment)
 * 
 * @param           pLuaState 
 */
extern CAPI void 
SLuaState_ClearGlobalTable(SLuaState* pLuaState);

/******************************************************************************
 * HEADER CALLBACK DECLARATIONS
 ******************************************************************************/

extern CAPI void
SCommand_ExecuteLuaBasedCommand(
    const struct SCommand* pSelf, 
    const struct SCommandArg* pArg
);

__END_C

#endif
