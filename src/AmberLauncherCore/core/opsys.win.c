#include <core/opsys.h>

#if defined(_WIN32) || defined(_WIN64)

#include <stdio.h>
#include <stdlib.h>

#include <windows.h>
#include <direct.h>

/******************************************************************************
 * HEADER FUNCTION DEFINITIONS
 ******************************************************************************/

CAPI void 
AmberLauncher_GetApplicationPath(char *pOutPath, size_t dSize)
{
    if (GetModuleFileNameA(NULL, pOutPath, (DWORD)dSize) == 0) 
    {
        fprintf(
            stderr,    
            "AmberLauncher_GetApplicationPath(char* pOutPath, size_t dSize) -> "
            "Error getting application path\n"
        );
    }
}

CAPI void 
AmberLauncher_GetApplicationFolder(char *pOutPath, size_t dSize)
{
    char path[1024];
    char* last_separator;
    AmberLauncher_GetApplicationPath(path, sizeof(path));

    last_separator = strrchr(path, '\\');

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
                "S_GetApplicationFolder(char* pPath,  size_t dSize) -> "
                "Buffer too small\n"
            );
        }
    } 
    else 
    {
        fprintf(
            stderr, 
            "AmberLauncher_GetApplicationFolder(char* pPath,  size_t dSize) -> "
            "No directory separator found in path\n"
        );
    }
}

CAPI void 
AmberLauncher__GetApplicationWorkingFolder(char *pOutPath, size_t dSize)
{
    if (_getcwd(pOutPath, (int)dSize) == NULL) 
    {
        fprintf(
            stderr, 
            "AmberLauncher_GetApplicationWorkingFolder(char* pOutPath,  size_t dSize) ->"
            "Error getting working folder\n"
        );
    }
}

CAPI CBOOL
AmberLauncher_IsRunningUnderWine(void)
{
    HKEY hKey;
    LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Wine", 0, KEY_READ, &hKey);
    if (result == ERROR_SUCCESS) 
    {
        RegCloseKey(hKey);
        return CTRUE; 
    }

    return CFALSE;
}

CAPI unsigned long
AmberLauncher_GetThreadID(void)
{
    return GetCurrentThreadId();
}

CAPI void
AmberLauncher_PrintDefaultSystemInformation(void)
{
    LPOSVERSIONINFO osvi;
    SYSTEM_INFO si;
    DWORD size;
    char computerName[MAX_COMPUTERNAME_LENGTH + 1];
    char versionStr[128];
    char architecture[32];

    ZeroMemory(&osvi, sizeof(LPOSVERSIONINFO));
    osvi->dwOSVersionInfoSize = sizeof(LPOSVERSIONINFO);
    if (!GetVersionEx(osvi)) 
    {
        fprintf(stderr, "Failed to retrieve OS version information");
        return;
    }

    ZeroMemory(&si, sizeof(SYSTEM_INFO));
    GetSystemInfo(&si);

    size = sizeof(computerName) / sizeof(computerName[0]);
    if (!GetComputerNameA(computerName, &size)) 
    {
        fprintf(stderr, "Failed to retrieve computer name");
        return;
    }

    /** Print the retrieved information */
    fprintf(stderr, "\tSystem name: \t%s", "Windows");  /** Windows doesn't have a direct equivalent to sysname */
    fprintf(stderr, "\tNode name: \t\t%s", computerName);
    fprintf(stderr, "\tRelease: \t\t%s",   "-");
    fprintf(stderr, "\tVersion: \t\t%s",   "-");
    fprintf(stderr, "\tMachine: \t\t%s",   "-");

    /** Construct version string */
    sprintf(versionStr, "Version %ld.%ld (Build %ld)", osvi->dwMajorVersion, osvi->dwMinorVersion, osvi->dwBuildNumber);
    fprintf(stderr, "\tOS Version: \t%s", versionStr);

    /** Determine architecture */
    switch (si.wProcessorArchitecture) 
    {
        case PROCESSOR_ARCHITECTURE_AMD64:
            strcpy(architecture, "x64 (AMD or Intel)");
            break;
        case PROCESSOR_ARCHITECTURE_ARM:
            strcpy(architecture, "ARM");
            break;
        case PROCESSOR_ARCHITECTURE_IA64:
            strcpy(architecture, "Intel Itanium-based");
            break;
        case PROCESSOR_ARCHITECTURE_INTEL:
            strcpy(architecture, "x86");
            break;
        default:
            strcpy(architecture, "Unknown architecture");
            break;
    }
    fprintf(stderr, "\tArchitecture: \t%s", architecture);
}

CAPI int
AmberLauncher_ProcessLaunch(const char* sAppPath, int argc, char **argv, CBOOL bCloseOnLaunch)
{
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    char cmdLine[MAX_PATH];

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    snprintf(cmdLine, sizeof(cmdLine), "\"%s\"", sAppPath);

    if (CreateProcessA(
        NULL,
        cmdLine,
        NULL, NULL,
        FALSE, 0,
        NULL, NULL,
        &si, &pi))
    {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return 1;
    }

    fprintf(stderr, "CreateProcess failed (%d): %s\n", GetLastError(), sAppPath);
    return 0;
}

CAPI int
AmberLauncher_SetRegistryKey(const char* sValueName, uint32 dValueData)
{
    HKEY hKey;
    const char* subKey = "Software\\New World Computing\\Might and Magic VII\\1.0";
    LONG result;

    result = RegOpenKeyExA(
        HKEY_LOCAL_MACHINE,
        subKey, 
        0, 
        KEY_SET_VALUE, 
        &hKey
    );

    if (result != ERROR_SUCCESS) 
    {
        fprintf(
            stderr, 
            "Failed to open 64-bit registry key, trying 32-bit view: %ld\n", 
            result
        );
        
        /* Try opening the 32-bit view if 64-bit access fails */
        result = RegOpenKeyExA(
            HKEY_LOCAL_MACHINE,
            subKey,
            0,
            KEY_SET_VALUE | KEY_WOW64_32KEY,
            &hKey
        );

        if (result != ERROR_SUCCESS) 
        {
            fprintf(stderr, "Error opening 32-bit registry key: %ld\n", result);
            return 1; 
        }
    }

    result = RegSetValueExA(
        hKey,
        sValueName,
        0,
        REG_DWORD,
        (const BYTE*)&dValueData,
        sizeof(DWORD)
    );

    if (result != ERROR_SUCCESS) 
    {
        fprintf(
            stderr, 
            "Error setting registry value '%s': %ld\n", 
            sValueName, 
            result
        );

        RegCloseKey(hKey);
        return 1;
    }

    fprintf(stderr, "Registry value '%s' changed successfully.\n", sValueName);

    RegCloseKey(hKey);
    return 0;
}

CAPI int
AmberLauncher_GetRegistryKey(const char* sValueName, uint32* pValueData)
{
    HKEY hKey;
    const char* subKey = "Software\\New World Computing\\Might and Magic VII\\1.0";
    LONG result;

    result = RegOpenKeyExA(
        HKEY_LOCAL_MACHINE,
        subKey,
        0,
        KEY_QUERY_VALUE,
        &hKey
    );

    if (result != ERROR_SUCCESS) 
    {
        fprintf(stderr, "Failed to open 64-bit registry key, trying 32-bit view: %ld\n", result);
        
        /* Try opening the 32-bit view if 64-bit access fails */
        result = RegOpenKeyExA(
            HKEY_LOCAL_MACHINE,
            subKey,
            0,
            KEY_QUERY_VALUE | KEY_WOW64_32KEY,
            &hKey
        );

        if (result != ERROR_SUCCESS) 
        {
            fprintf(stderr, "Error opening 32-bit registry key: %ld\n", result);
            return 1; 
        }
    }

    DWORD dataType = REG_DWORD;
    DWORD dataSize = sizeof(DWORD);
    DWORD valueData = 0;

    result = RegQueryValueExA(
        hKey,
        sValueName,
        NULL,
        &dataType,
        (LPBYTE)&valueData,
        &dataSize
    );

    if (result != ERROR_SUCCESS) 
    {
        fprintf(stderr, "Error querying registry value '%s': %ld\n", sValueName, result);
        RegCloseKey(hKey);
        return 1;
    }

    if (dataType != REG_DWORD || dataSize != sizeof(DWORD)) 
    {
        fprintf(stderr, "Registry value '%s' has unexpected type or size.\n", sValueName);
        RegCloseKey(hKey);
        return 1;
    }

    *pValueData = valueData;

    fprintf(stderr, "Registry value '%s' retrieved successfully: %u.\n", sValueName, valueData);

    RegCloseKey(hKey);
    return 0;
}

#endif
