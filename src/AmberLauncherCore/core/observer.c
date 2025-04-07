#include "core/vector.h"
#include <core/observer.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/******************************************************************************
 * HEADER FUNCTION DEFINITIONS
 ******************************************************************************/

/******************************************************************************
 * OBSERVER
 ******************************************************************************/

extern CAPI SObserver*
SObserver_new(void)
{
    SObserver* pObserver = malloc(sizeof(SObserver));
    if (!IS_VALID(pObserver))
    {
        fprintf
        (
            stderr,
            "SObserver_new() -> Failed to allocate memory."
        );

        return NULL;
    }

    pObserver->pOwner   = NULL;
    pObserver->cbUpdate = NULL;

    return pObserver;
}

extern CAPI CBOOL
SObserver_delete(SObserver** pObserver)
{
    if (!IS_VALID(pObserver) || !IS_VALID(*pObserver))
    {
        fprintf
        (
            stderr,
            "SObserver_delete(SObserver** pObserver) -> "
            "received \"pObserver\" as NULL."
        );
        return CFALSE;
    }

    (*pObserver)->pOwner = NULL;

    free(*pObserver);
    *pObserver = NULL;

    return CTRUE;
}

/******************************************************************************
 * SUBJECT
 ******************************************************************************/

CAPI SSubject*
SSubject_new(void)
{
    SSubject* pSubject = malloc(sizeof(SSubject));
    if (!IS_VALID(pSubject))
    {
        fprintf
        (
            stderr,
            "SSubject_new() -> Failed to allocate memory."
        );

        return NULL;
    }

    /* Initialize variables */
    SVector_Init(&pSubject->tObservers, sizeof(SObserver));
    SVector_Reserve(&pSubject->tObservers, OBSERVER_DEFAULT_SIZE);

    return pSubject;
}

CAPI CBOOL
SSubject_delete(SSubject** pSubject)
{
    if (!IS_VALID(pSubject) || !IS_VALID(*pSubject))
    {
        fprintf
        (
            stderr,
            "SSubject_delete(SSubject** pSubject) -> "
            "received \"pSubject\" as NULL."
        );
        return CFALSE;
    }

    SVector_Cleanup(&(*pSubject)->tObservers);
    free(*pSubject);
    *pSubject = NULL;

    return CTRUE;
}

CAPI void
SSubject_Attach(SSubject *pSubject, SObserver *pObserver)
{
    assert(IS_VALID(pSubject));
    assert(IS_VALID(pObserver));

    SVector_PushBack(&pSubject->tObservers, pObserver);
}

CAPI void
SSubject_Detach(SSubject *pSubject, SObserver *pObserver)
{
    long int found;

    assert(IS_VALID(pSubject));
    assert(IS_VALID(pObserver));

    found = SVector_FindByValue(&pSubject->tObservers, pObserver);

    if (found != -1)
    {
        SVector_Erase(&pSubject->tObservers, (size_t)found);
    }
}

CAPI void
SSubject_Notify(SSubject *pSubject, int32 dFlags, void* pUserData)
{
    assert(IS_VALID(pSubject));

    SVector_ForEach(&pSubject->tObservers)
    {
        SVector_InitIterator(SObserver, &pSubject->tObservers);
        SVECTOR_ITERATOR->cbUpdate(SVECTOR_ITERATOR, dFlags, pUserData);
    }
}
