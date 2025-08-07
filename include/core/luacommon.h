#ifndef SLUACOMMON_H_
#define SLUACOMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * FORWARD DECLARATIONS
 ******************************************************************************/

/******************************************************************************
 * ENUMS
 ******************************************************************************/

/**
 * @brief   Reference index to lua function from ref array in SLuaState
 * 
 */
typedef enum 
{
    SLUA_FUNC_APPINIT,
    SLUA_FUNC_POST_APPINIT,
    SLUA_FUNC_APPDESTROY,
    SLUA_FUNC_POST_APPDESTROY,
    SLUA_FUNC_APPCONFIGURE,
    SLUA_FUNC_POST_APPCONFIGURE,
    SLUA_FUNC_MAX

} ELuaFunctionRefType;

extern const char* ELuaFunctionRefTypeStrings[];
/**
 * @brief   Predefined strings for 'function events.{string}' callbacks
 * 
 */
typedef enum 
{
    SLUA_EVENT_INIT,
    SLUA_EVENT_DESTROY,
    SLUA_EVENT_CONFIGURE_BEFORE,
    SLUA_EVENT_CONFIGURE_AFTER,
    SLUA_EVENT_PLAY,
    SLUA_EVENT_MAX

} ELuaFunctionEventType;

extern const char* ELuaFunctionEventTypeStrings[];

/**
 * @brief   Variable type that Lua state accepts
 * 
 */
typedef enum 
{
    SLUA_TYPE_NIL,      /*!< Null */
    SLUA_TYPE_BOOL,     /*!< Boolean */
    SLUA_TYPE_NUMBER,   /*!< Decimal */
    SLUA_TYPE_STRING,   /*!< String */
    SLUA_TYPE_USERDATA, /*!< User data (anything) */
    SLUA_TYPE_FUNC,     /*!< Function */
    SLUA_TYPE_THREAD,   /*!< Thread */
    SLUA_TYPE_TABLE,    /*!< Lua Table */
    SLUA_TYPE_MAX   

} ELuaVarType;

/******************************************************************************
 * STRUCTS
 ******************************************************************************/

/**
 * @brief   Wrapper for Lua Variable
 * 
 */
typedef struct 
{
    ELuaVarType type;
    union 
    {
        double num;
        int boolean; /** @todo where's integer?  */
        const char* str;
    } data;
    void* userdata;
} SLuaVar;

/**
 * @brief   Wrapper for Lua References
 * 
 */
typedef struct SLuaFunctionRef 
{
    int             dRef;   /**< Reference ID from Lua state */
    const char*     sName;  /**< String key for Lua Ref array */

} SLuaFunctionRef;

#ifdef __cplusplus
}
#endif

#endif
