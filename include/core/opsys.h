#ifndef OPERATING_SYSTEM_H_
#define OPERATING_SYSTEM_H_

#include <core/common.h>
#include <stddef.h>

/******************************************************************************
 * PREPROCESSOR
 ******************************************************************************/

#define STR_DEFAULT_BUFFER_SIZE 512
#define MAX_LINE_LENGTH 1024

/******************************************************************************
 * HEADER FUNCTION DECLARATIONS
 ******************************************************************************/

/**
 * @relatedalso AmberLauncher
 * @brief       Returns bitness value of this app
 * 
 * @return      int 
 */
extern CAPI int 
AmberLauncher_GetApplicationBitness(void);

/**
 * @relatedalso AmberLauncher
 * @brief       Get full path to application
 * 
 * @param pPath Return path
 * @param dSize 
 */
extern CAPI void 
AmberLauncher_GetApplicationPath(char *pOutPath, size_t dSize);

/**
 * @relatedalso AmberLauncher
 * @brief       Get full path to application folder
 * 
 * @param pPath Return path
 * @param dSize 
 */
extern CAPI void 
AmberLauncher_GetApplicationFolder(char *pOutPath, size_t dSize);

/**
 * @relatedalso AmberLauncher
 * @brief       Get full working directory path
 * 
 * @param pPath Return path
 * @param dSize 
 */
extern CAPI void 
AmberLauncher_GetApplicationWorkingFolder(char *pOutPath, size_t dSize);

/**
 * @relatedalso AmberLauncher
 * @brief       Returns true if system is little-endian
 * 
 * @return      CBOOL 
 */
extern CAPI CBOOL 
AmberLauncher_IsLittleEndian(void);

/**
 * @relatedalso AmberLauncher
 * @brief       Checks if application is running via WINE (unix systems)
 * 
 * @return      CBOOL 
 */
extern CAPI CBOOL
AmberLauncher_IsRunningUnderWine(void);

/**
 * @relatedalso AmberLauncher
 * @brief       Returns OS-specific main thread ID
 * 
 * @return      unsigned long 
 */
extern CAPI unsigned long
AmberLauncher_GetThreadID(void);

/**
 * @relatedalso AmberLauncher
 * @brief       Default OS-Specific system information dump
 * 
 */
extern CAPI void
AmberLauncher_PrintDefaultSystemInformation(void);

/**
 * @relatedalso AmberLauncher
 * @brief       Launches separate process
 * 
 * @param       sAppPath 
 * @param       argc
 * @param       argv
 * @param       bCloseOnLaunch
 * @return      int Success
 */
extern CAPI int
AmberLauncher_ProcessLaunch(
    const char* sAppPath, 
    int argc, 
    char **argv, 
    CBOOL bCloseOnLaunch
);

extern CAPI int
AmberLauncher_SetRegistryKey(const char* sValueName, uint32 dValueData);

extern CAPI int
AmberLauncher_GetRegistryKey(const char* sValueName, uint32* pValueData);

#ifdef __cplusplus
}
#endif

#endif
