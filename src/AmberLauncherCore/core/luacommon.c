#include <core/luacommon.h>

#include <stdio.h>

const char* ELuaFunctionRefTypeStrings[] = {
    "OnAppInit",
    "OnAppScriptsLoaded",
    "OnGameReset",
    "OnGameLoad",
    "OnGamePrepare",
    "OnGameReady",
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
    "GameActivated",
    "GameDeactivated",
    "BeforeConfigure",
    "AfterConfigure",
    "PlayGame",
    "BeforeUpdate",
    "AfterUpdate",
    NULL
};
