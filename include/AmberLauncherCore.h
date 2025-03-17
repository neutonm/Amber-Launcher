#ifndef __AMBER_LAUNCHER_CORE_H
#define __AMBER_LAUNCHER_CORE_H

#include <core/common.h>

__EXTERN_C

/******************************************************************************
 * FORWARD DECLARATION
 ******************************************************************************/

struct SLuaState;

/******************************************************************************
 * STRUCTS
 ******************************************************************************/

typedef struct ALContext_t
{
    struct SLuaState* pLuaState;
} ALContext;

/******************************************************************************
 * HEADER DECLARATIONS
 ******************************************************************************/

extern CAPI void
AmberLauncher_Start(void);

extern CAPI void
AmberLauncher_End(void);

extern CAPI void
AmberLauncher_Test(App* pApp);

extern CAPI void
AmberLauncher_ConfigureStart(App* pApp);

extern CAPI void
AmberLauncher_Play(App* pApp);

__END_C

#endif
