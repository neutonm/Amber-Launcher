#ifndef __AMBER_LAUNCHER_APPCORE_H_
#define __AMBER_LAUNCHER_APPCORE_H_

#include <core/common.h>

/******************************************************************************
 * FORWARD DECLARATION
 ******************************************************************************/

 struct SLuaState;
 struct SSubject;

 /******************************************************************************
  * STRUCTS
  ******************************************************************************/

typedef struct AppCore AppCore;
typedef SVarKeyBundle (*UICallback)(
  AppCore *pAppCore,
  uint32 ID,
  const SVar *pUserData,
  const unsigned int dNumArgs);

struct AppCore
{
    int argc;
    char **argv;
    void *pOwner;
    struct SLuaState* pLuaState;
    struct SSubject* pOnUserEventNotifier;
    UICallback cbUIEvent;
    char *sLaunchCmd;
};

/******************************************************************************
 * HEADER DECLARATIONS
 ******************************************************************************/

__EXTERN_C

extern CAPI AppCore*
AppCore_create(void);

extern CAPI CBOOL
AppCore_init(AppCore* pAppCore);

extern CAPI CBOOL
AppCore_free(AppCore** pAppCore);

extern CAPI CBOOL
AppCore_SetLaunchCommand(AppCore* pAppCore, const char *sNewLaunchCmd);

extern CAPI const char*
AppCore_GetDefaultLaunchCommand(void);

__END_C

#endif
