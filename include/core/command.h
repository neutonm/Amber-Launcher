#ifndef SCOMMAND_H_
#define SCOMMAND_H_

#include <core/common.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * FORWARD DECLARATIONS
 ******************************************************************************/

struct lua_State;
typedef struct SCommand SCommand;

/******************************************************************************
 * MACROS
 ******************************************************************************/

/**
 * @brief Flags for SCommand.dFlags
 * 
 */
#define SCOMMAND_FLAG_NULL              0x00 /**< Improves readibility */
#define SCOMMAND_FLAG_PREINIT           0x01 /**< Pre-app init processing */
#define SCOMMAND_FLAG_POSTINIT          0x02 /**< Process this command at the end of function */
#define SCOMMAND_FLAG_CLEANUP           0x04 /**< Processes inside CEngine_delete */
#define SCOMMAND_FLAG_LUA               0x08 /**< Created using lua */

/******************************************************************************
 * STRUCTS
 ******************************************************************************/

/**
 * @brief   Substitute for void* argument ("variant")
 * 
 */
typedef struct SCommandArg 
{
    union 
    {
        int32 dValue;
        char* sValue;
        void* pValue;
    } uData;
    uint32 dFlags;
    ECType eType; 
    
} SCommandArg;

typedef CBOOL (*CommandFunc)(const SCommand* pSelf, const SCommandArg *pArgs, const unsigned int dNumArgs);

/**
 * @brief   Command pattern realization
 * 
 *          Specially designed to represent player's potential actions.
 * 
 */
struct SCommand 
{
    const char*     sName;          /**< String key for CCommand array */
    void*           pOwner;         /**< Owner of the command */
    CommandFunc     cbExecuteFunc;  /**< Callback function */
    uint32          dFlags;         /**< Custom options for callback as flags  */
    int8            dNumArgs;       /**< Number of arguments inside SCommandArg */
    int             dLuaRef;        /**< Lua Reference */
    int             dPriority;      /**< Priority (for internal sorting) */
};

/******************************************************************************
 * HEADER FUNCTION DECLARATIONS
 ******************************************************************************/

/**
 * @relatedalso SCommand
 * @brief       Allocates memory for command and sets its defaults values
 * 
 * @return      SCommand*
 */
extern CAPI SCommand*
SCommand_new(void);

/**
 * @relatedalso SCommand
 * @brief       Destroys and cleans up command
 * 
 * @param       pCommand 
 * @return      CBOOL Success 
 */
extern CAPI CBOOL
SCommand_delete(SCommand** pCommand);

/**
 * @relatedalso SCommand
 * @brief       Sets Scommand.dFlags flag
 * 
 * @param       pCommand 
 * @param       dFlag 
 * @return      Success
 */
extern CAPI CBOOL
SCommand_SetFlag(SCommand* pCommand, unsigned int dFlag);

/**
 * @relatedalso SCommand
 * @brief       Checks if flag is set inside SCommand.dFlags
 * 
 * @param       pCommand 
 * @param       dFlag 
 */
extern CAPI CBOOL
SCommand_IsFlagSet(const SCommand* pCommand, unsigned int dFlag);

/**
 * @relatedalso SCommand
 * @brief       Clears SCommand.dFlags flag
 * 
 * @param       pCommand 
 * @param       dFlag 
 */
extern CAPI void
SCommand_ClearFlag(SCommand* pCommand, unsigned int dFlag);

/**
 * @relatedalso SCommandArg
 * @brief       Makes CCommandArg with intValue = 0;
 * 
 * @return      CCommandArg 
 */
extern CAPI SCommandArg 
SCommandArg_MakeNull(void);

/**
 * @relatedalso SCommandArg
 * @brief       Makes CCommandArg with dValue = dVar;
 * 
 * @param       dVar
 * @return      CCommandArg 
 */
extern CAPI SCommandArg 
SCommandArg_MakeInt(int dVar);

/**
 * @relatedalso SCommandArg
 * @brief       Makes CCommandArg with sValue = sVar;
 * 
 * @param       sVar
 * @return      CCommandArg 
 */
extern CAPI SCommandArg 
SCommandArg_MakeString(char* sVar);

/**
 * @relatedalso SCommandArg
 * @brief       Makes CCommandArg with pValue = pVar;
 * 
 * @param       pVar
 * @return      CCommandArg 
 */
extern CAPI SCommandArg 
SCommandArg_MakeVoid(void* pVar);

extern CAPI int
LUA_RegisterCommand(struct lua_State *L);

#ifdef __cplusplus
}
#endif

#endif
