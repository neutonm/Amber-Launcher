#include <core/luacommon.h>

#include <stdio.h>

const char* ELuaFunctionRefTypeStrings[] = {
    "OnAppInit",
    "OnPostAppInit",
    "OnAppDestroy",
    "OnPostAppDestroy",
    "OnAppConfigure",
    "OnPostAppConfigure",
    "OnPlay",
    "OnSidebuttonClick",
    "OnUpdateStart",
    "OnUpdateEnd",
    NULL
};

const char* ELuaFunctionEventTypeStrings[] = {
    "InitLauncher",
    "DestroyLauncher",
    "BeforeConfigure",
    "AfterConfigure",
    "PlayGame",
    "BeforeUpdate",
    "AfterUpdate",
    NULL
};
