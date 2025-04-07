#ifndef CAPPCORE_H_
#define CAPPCORE_H_

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
typedef SVar (*UICallback)(AppCore *pAppCore, uint32 ID, SVar *pUserData);

struct AppCore
{
    int argc;
    char **argv;
    void *pOwner;
    struct SLuaState* pLuaState;
    struct SSubject* pOnUserEventNotifier;
    UICallback cbUIEvent;
};

/******************************************************************************
 * HEADER DECLARATIONS
 ******************************************************************************/

extern CAPI AppCore*
AppCore_create(void);

extern CAPI CBOOL
AppCore_init(AppCore* pAppCore);

extern CAPI CBOOL
AppCore_free(AppCore** pAppCore);

#endif
