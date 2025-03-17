#include <core/luacommon.h>

#include <stdio.h>

const char* ELuaFunctionRefTypeStrings[] = {
    "AppInit",
    "AppDestroy",
    "AppConfigure",
    NULL
};

const char* ELuaFunctionEventTypeStrings[] = {
    "InitLauncher",
    "DestroyLauncher",
    "BeforeConfigure",
    "AfterConfigure",
    "PlayGame",
    NULL
};
