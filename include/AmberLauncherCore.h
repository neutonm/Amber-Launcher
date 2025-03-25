#ifndef __AMBER_LAUNCHER_CORE_H
#define __AMBER_LAUNCHER_CORE_H

#include <core/common.h>

__EXTERN_C

/******************************************************************************
 * FORWARD DECLARATION
 ******************************************************************************/

struct AppCore;

/******************************************************************************
 * HEADER DECLARATIONS
 ******************************************************************************/

extern CAPI void
AmberLauncher_Start(struct AppCore* pApp);

extern CAPI void
AmberLauncher_End(struct AppCore* pApp);

extern CAPI void
AmberLauncher_Test(struct AppCore* pApp);

extern CAPI void
AmberLauncher_ConfigureStart(struct AppCore* pApp);

extern CAPI void
AmberLauncher_Play(struct AppCore* pApp);

__END_C

#endif
