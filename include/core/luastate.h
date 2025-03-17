#ifndef SLUASTATE_H_
#define SLUASTATE_H_

#include <core/common.h>
#include <core/vector.h>
#include <core/luacommon.h>

#ifdef __cplusplus
extern "C" {
#endif

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
 * @return          bool_t 
 */
extern CAPI bool_t 
SLuaState_delete(SLuaState** pLuaState);

/**
 * @relatedalso     SLuaState
 * @brief           Loads default scripts
 * 
 * @param           pLuaState 
 * @return          bool_t 
 */
extern CAPI bool_t
SLuaState_Init(SLuaState* pLuaState);

/**
 * @relatedalso     SLuaState
 * @brief           Registers function reference in lua state inside SLuaState's 
 *                  RefFunctions array;
 * 
 * @param           pLuaState 
 * @param           eRefType 
 * @param           sLuaFunctionName 
 * @return          bool_t 
 */
extern CAPI bool_t 
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
 * @return          bool_t 
 */
extern CAPI bool_t
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
 * @return          bool_t
 */
extern CAPI bool_t 
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
 * @return          bool_t 
 */
extern CAPI bool_t 
SLuaVar_IsOfType(const SLuaVar* pVar, ELuaVarType eType);

/**
 * @relatedalso     SLuaState
 * @brief           Executes Lua code
 * 
 * @param           pLuaState 
 * @param           sCode 
 */
extern CAPI bool_t 
SLuaState_ExecuteCode(SLuaState* pLuaState, const char *sCode);

/**
 * @relatedalso     SLuaState
 * @brief           Loads and Executes Lua Script
 * 
 * @param           pLuaState 
 * @param           sScriptPath 
 * @return          bool_t 
 */
extern CAPI bool_t 
SLuaState_LoadScript(SLuaState* pLuaState, const char* sScriptPath);

/**
 * @relatedalso     SLuaState
 * @brief           Calls registered referenced lua function state from SLuaState's 
 *                  RefFunctions array
 *
 * @param           pLuaState 
 * @param           eRefType 
 * @return          bool_t 
 */
extern CAPI bool_t 
SLuaState_CallReferencedFunction(
    SLuaState* pLuaState, 
    ELuaFunctionRefType eRefType
);

extern CAPI void 
SLuaState_CallEvent(SLuaState* pLuaState, const char* sEventName);

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

#ifdef __cplusplus
}
#endif

#endif
