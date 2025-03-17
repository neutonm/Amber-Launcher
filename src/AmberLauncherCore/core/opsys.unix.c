
#include <core/opsys.h>

#if defined(__unix__)

#if defined(__linux__)
#   define _GNU_SOURCE
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <sys/utsname.h>
#endif

#include <pthread.h>
#include <unistd.h>

#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#if _XOPEN_SOURCE >= 600 
#include <spawn.h>

extern char **environ;
#endif

#if defined(__GLIBC__)
#include <execinfo.h>
#endif

/******************************************************************************
 * STATIC FUNCTION DECLARATIONS
 ******************************************************************************/

static int
_AmberLauncher_ConcatCharArray(
    char* dest, 
    char **pArray, 
    int dNum, 
    int dSize, 
    char sSpaceChar
);

static int 
_AmberLauncher_IsWinePrefix64bit(const char* winePrefix);

/******************************************************************************
 * HEADER FUNCTION DEFINITIONS
 ******************************************************************************/

CAPI void 
AmberLauncher_GetApplicationPath(char *pOutPath, size_t dSize)
{
    ssize_t len = readlink("/proc/self/exe", pOutPath, dSize - 1);
    if (len == -1) 
    {
        fprintf(stderr, "Error getting application path");
    }
    pOutPath[len] = '\0';
}

CAPI void 
AmberLauncher_GetApplicationFolder(char *pOutPath, size_t dSize)
{
    char path[1024];
    char* last_separator;
    AmberLauncher_GetApplicationPath(path, sizeof(path));

    last_separator = strrchr(path, '/');

    if (last_separator != NULL) 
    {
        size_t len = (size_t)last_separator - (size_t)path;
        if (len < dSize) 
        {
            strncpy(pOutPath, path, len);
            pOutPath[len] = '\0';
        } 
        else 
        {
            fprintf(
                stderr,   
                "AmberLauncher_GetApplicationFolder(char* pPath,  size_t dSize"
                "Buffer too small\n"
            );
        }
    } 
    else 
    {
        fprintf(
            stderr,    
            "AmberLauncher_GetApplicationFolder(char* pPath,  size_t dSize"
            "No directory separator found in path\n"
        );
    }
}

CAPI void 
AmberLauncher_GetApplicationWorkingFolder(char *pOutPath, size_t dSize)
{
    if (getcwd(pOutPath, dSize) == NULL) 
    {
        fprintf(stderr,    
            "AmberLauncher_GetApplicationWorkingFolder(char* pOutPath,  size_t dSize) ->"
            "Error getting working folder\n"
        );
    }
}

CAPI bool_t
AmberLauncher_IsRunningUnderWine(void)
{
    return FALSE;
}

CAPI unsigned long
AmberLauncher_GetThreadID(void)
{
    #if defined(__linux__)
    return (unsigned long)syscall(SYS_gettid);
    #endif

    return (unsigned long)pthread_self();
}

CAPI void
AmberLauncher_PrintDefaultSystemInformation(void)
{
    struct utsname buffer;

    if (uname(&buffer) != 0) 
    {
        fprintf(stderr, "Failed to retrieve info via \"uname\"");
        return;
    }

    fprintf(stderr, "\tSystem name: \t%s",   buffer.sysname);
    fprintf(stderr, "\tNode name: \t\t%s",   buffer.nodename);
    fprintf(stderr, "\tRelease: \t\t%s",     buffer.release);
    fprintf(stderr, "\tVersion: \t\t%s",     buffer.version);
    fprintf(stderr, "\tMachine: \t\t%s",     buffer.machine);

    #ifdef _GNU_SOURCE
    fprintf(stderr, "\tDomain name: \t%s",   buffer.__domainname);
    #endif
}

CAPI int
AmberLauncher_ProcessLaunch(const char* sAppPath, int argc, char **argv, bool_t bCloseOnLaunch)
{
    pid_t dPID      = 0;
    int dStatus     = 0;

    /* posix_spawn method */
    #if _XOPEN_SOURCE >= 600 

        char *pAltArgv[] = {"", (char*)0};

        pAltArgv[0] = (char*)sAppPath;

        dStatus = posix_spawn(&dPID, sAppPath, NULL, NULL, argv == NULL ? pAltArgv : argv, environ);
        if(dStatus != 0) 
        {
            fprintf(stderr, "Failed to launch process: %s", strerror(dStatus));
            return 0;
        }

        if (bCloseOnLaunch)
        {
            exit(EXIT_SUCCESS);
        }
        
        UNUSED(argv);
        UNUSED(argc);
        
    #else

        /* the old way */
        char sArgvCombined[STR_DEFAULT_BUFFER_SIZE];

        if (access(sAppPath, X_OK) != 0) 
        {
            fprintf(stderr, "Executable doesn't exist: %s", sAppPath);
            return 0;
        }
        
        dPID = fork();
        _AmberLauncher_ConcatCharArray(sArgvCombined, argv, argc, STR_DEFAULT_BUFFER_SIZE, ' ');

        switch(dPID)
        {
            case -1:
                fprintf(stderr, "Failed to fork: %s", sAppPath);
                perror("fork");
                return 0;
            
            case 0:

                dStatus = execl(sAppPath, sArgvCombined, (char *)NULL);
                if (dStatus == -1)
                {
                    fprintf(stderr, "Failed to launch process: %s", sAppPath);
                    return 0;
                }
                return 1;

            default:

                if (bCloseOnLaunch)
                {
                    exit(EXIT_SUCCESS);
                }
                break;
        }

        /* waitdPID(dPID, NULL, 0); */

    #endif

    return dPID;
}

CAPI int
AmberLauncher_SetRegistryKey(const char* sValueName, uint32_t dValueData)
{
    const char *SECTION_64 = 
    "HKEY_LOCAL_MACHINE\\\\Software\\\\WOW6432Node\\\\New World Computing\\\\Might and Magic VII\\\\1.0";
    const char *SECTION_32 = 
    "HKEY_LOCAL_MACHINE\\\\Software\\\\New World Computing\\\\Might and Magic VII\\\\1.0";
    
    char *winePrefix;
    char regFilePath[MAX_LINE_LENGTH];
    char command[2048];

    winePrefix = getenv("WINEPREFIX");
    if (winePrefix == NULL) {
        winePrefix = getenv("HOME");
        if (winePrefix == NULL) {
            fprintf(stderr, "Unable to determine the user's home directory.\n");
            return 1;
        }
        snprintf(regFilePath, sizeof(regFilePath), "%s/.wine/", winePrefix);
        winePrefix = regFilePath;
    }

     snprintf(command, sizeof(command),
             "WINEPREFIX=\"%s\" wine reg add \"%s\" /v \"%s\" /t REG_DWORD /d %u /f",
             winePrefix,
             _AmberLauncher_IsWinePrefix64bit(winePrefix) ? SECTION_64 : SECTION_32,
             sValueName,
             dValueData);
    
    /* Execute the command */
    int ret = system(command);
    
    if (ret != 0) 
    {
        fprintf(stderr, "Failed to set registry key. Command executed: %s\n", command);
        return -1;
    }
    
    return 0;
}

CAPI int
AmberLauncher_GetRegistryKey(const char* sValueName, uint32_t* pValueData)
{
    const char *SECTION_64 = 
    "HKEY_LOCAL_MACHINE\\Software\\WOW6432Node\\New World Computing\\Might and Magic VII\\1.0";
    const char *SECTION_32 = 
    "HKEY_LOCAL_MACHINE\\Software\\New World Computing\\Might and Magic VII\\1.0";
    
    char *winePrefix;
    char regFilePath[MAX_LINE_LENGTH];
    char command[2048];
    char buffer[1024];
    char line[1024];
    char name[256], type[256], data[256];

    winePrefix = getenv("WINEPREFIX");
    if (winePrefix == NULL) {
        winePrefix = getenv("HOME");
        if (winePrefix == NULL) {
            fprintf(stderr, "Unable to determine the user's home directory.\n");
            return 1;
        }
        snprintf(regFilePath, sizeof(regFilePath), "%s/.wine", winePrefix);
        winePrefix = regFilePath;
    }

    const char* registrySection = _AmberLauncher_IsWinePrefix64bit(winePrefix) ? SECTION_64 : SECTION_32;

    snprintf(command, sizeof(command),
             "WINEPREFIX=\"%s\" wine reg query \"%s\" /v \"%s\" 2>/dev/null",
             winePrefix,
             registrySection,
             sValueName);

    /* Execute the command and read the output */
    FILE* fp = popen(command, "r");
    if (!fp) {
        fprintf(stderr, "Failed to run command: %s\n", command);
        return 1;
    }

    uint32_t valueData = 0;
    int found = 0;

    while (fgets(line, sizeof(line), fp) != NULL) {
        // Remove the newline character at the end, if any
        line[strcspn(line, "\n")] = '\0';

        // Skip lines that don't contain "REG_"
        if (strstr(line, "REG_") == NULL) {
            continue;
        }

        // Trim leading whitespace
        char *p = line;
        while (*p == ' ' || *p == '\t') {
            p++;
        }

        // Find the "REG_" in the line
        char *pType = strstr(p, "REG_");
        if (pType == NULL) {
            continue; // Should not happen, since we checked earlier
        }

        // The "name" is from p to pType - 1, trimming trailing whitespace
        char *pNameEnd = pType;
        while (pNameEnd > p && (*(pNameEnd - 1) == ' ' || *(pNameEnd - 1) == '\t')) {
            pNameEnd--;
        }
        size_t nameLen = pNameEnd - p;
        if (nameLen >= sizeof(name)) {
            nameLen = sizeof(name) - 1;
        }
        strncpy(name, p, nameLen);
        name[nameLen] = '\0';

        // The "type" is from pType to the next whitespace or end of string
        char *pTypeEnd = pType;
        while (*pTypeEnd != ' ' && *pTypeEnd != '\t' && *pTypeEnd != '\0') {
            pTypeEnd++;
        }
        size_t typeLen = pTypeEnd - pType;
        if (typeLen >= sizeof(type)) {
            typeLen = sizeof(type) - 1;
        }
        strncpy(type, pType, typeLen);
        type[typeLen] = '\0';

        // The "data" is the rest of the line after pTypeEnd, skipping leading whitespace
        char *pData = pTypeEnd;
        while (*pData == ' ' || *pData == '\t') {
            pData++;
        }
        strncpy(data, pData, sizeof(data) - 1);
        data[sizeof(data) - 1] = '\0';

        // Now you have name, type, and data
        if (strcmp(name, sValueName) == 0 && strcmp(type, "REG_DWORD") == 0) {
            valueData = (uint32_t)strtoul(data, NULL, 0);
            *pValueData = valueData;
            found = 1;
            break;
        }
    }

    pclose(fp);

    if (!found) {
        fprintf(stderr, "Failed to retrieve registry value '%s'.\n", sValueName);
        return 1;
    }

    fprintf(stderr, "Registry value '%s' retrieved successfully: %u.\n", sValueName, valueData);

    return 0;
}

/******************************************************************************
 * STATIC FUNCTION DEFINITIONS
 ******************************************************************************/

static int
_AmberLauncher_ConcatCharArray(
    char* dest, 
    char **pArray, 
    int dNum, 
    int dSize, 
    char sSpaceChar
)
{
    int i;
    size_t dCastSize = (size_t)dSize; /* for strlen */

    if (dest == NULL || pArray == NULL || dNum == 0 || dSize == 0)
    {
        return 1;
    }
    
    /* Init buffer */
    dest[0] = '\0';

    for (i = 0; i < dNum; ++i) {

        if (strlen(dest) + strlen(pArray[i]) + 1 >= dCastSize) 
        { 
            fprintf(stderr, "Error: concatenated arguments exceed buffer size!\n");
            return 1;
        }

        strcat(dest, pArray[i]);

        /* Add a space between arguments, but not after the last one */
        if (sSpaceChar != '\0' && i < dSize - 1) 
        {
            size_t len = strlen(dest);
            if (len + 1 >= dCastSize) 
            {
                fprintf(stderr, "Error: concatenated arguments exceed buffer size!\n");
                return 1;
            }
            dest[len] = sSpaceChar;
            dest[len + 1] = '\0';
        }
    }

    return 0;
}

static int 
_AmberLauncher_IsWinePrefix64bit(const char* winePrefix)
{
    FILE *regFile;
    char regFilePath[1024];
    char line[1024];

    /* Construct the path to the system.reg file */
    snprintf(regFilePath, sizeof(regFilePath), "%s/system.reg", winePrefix);

    /* Open the system.reg file */
    regFile = fopen(regFilePath, "r");
    if (regFile == NULL) {
        fprintf(stderr, "Unable to open Wine registry file for detection: %s\n", regFilePath);
        return 0; /* Default to 32-bit if detection fails */
    }

    /* Check for the #arch line to determine the architecture */
    while (fgets(line, sizeof(line), regFile) != NULL) {
        /* Look for the line containing '#arch' */
        if (strstr(line, "#arch") != NULL) {
            /* Extract the architecture value */
            if (strstr(line, "win64") != NULL) {
                fclose(regFile);
                return 1; /* 64-bit Wine prefix detected */
            } else if (strstr(line, "win32") != NULL) {
                fclose(regFile);
                return 0; /* 32-bit Wine prefix detected */
            }
        }
    }

    /* Close the file */
    fclose(regFile);

    /* Default to 32-bit if unable to detect the architecture */
    return 0;
}

#endif
