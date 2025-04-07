#ifndef SOBSERVER_H_
#define SOBSERVER_H_

#include <core/vector.h>
#include <core/common.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * MACROS
 ******************************************************************************/

#define OBSERVER_DEFAULT_SIZE 16

/******************************************************************************
 * STRUCTS
 ******************************************************************************/

typedef struct SObserver 
{
    void *pOwner;
    void (*cbUpdate)(struct SObserver *pSelf, int32 dFlags, void *pUserData);
} SObserver;

typedef struct SSubject
{
    SVECTOR_DECLARE(tObservers, SObserver);
} SSubject;

/******************************************************************************
 * HEADER FUNCTION DECLARATIONS
 ******************************************************************************/

/* SObserver */
extern CAPI SObserver*
SObserver_new(void);

extern CAPI CBOOL
SObserver_delete(SObserver** pObserver);

/* SSubject */
extern CAPI SSubject*
SSubject_new(void);

extern CAPI CBOOL 
SSubject_delete(SSubject** pSubject);

extern CAPI void
SSubject_Attach(SSubject *pSubject, SObserver *pObserver);

extern CAPI void
SSubject_Detach(SSubject *pSubject, SObserver *pObserver);

extern CAPI void
SSubject_Notify(SSubject *pSubject, int32 dFlags, void *pUserData);

#ifdef __cplusplus
}
#endif

#endif
