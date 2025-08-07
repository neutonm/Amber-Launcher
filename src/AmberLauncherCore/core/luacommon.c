#include <core/luacommon.h>

#include <stdio.h>

const char* ELuaFunctionRefTypeStrings[] = {
    "AppInit",
    "PostAppInit",
    "AppDestroy",
    "PostAppDestroy",
    "AppConfigure",
    "PostAppConfigure",
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
