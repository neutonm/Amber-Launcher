#ifndef CAPPCORE_H_
#define CAPPCORE_H_

#include <core/common.h>

/******************************************************************************
 * FORWARD DECLARATION
 ******************************************************************************/

 struct SLuaState;

 /******************************************************************************
  * STRUCTS
  ******************************************************************************/

typedef struct AppCore
{
    int argc;
    char** argv;
    struct SLuaState* pLuaState;
} AppCore;

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
