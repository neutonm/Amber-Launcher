
#include <core/vector.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/** 
 * @todo stuff that could be implemented: 
 * shrink_to_fit, begin/end, sort (with callback), error checks /asserts, serialization, 
 */

/******************************************************************************
 * HEADER FUNCTION DEFINITIONS
 ******************************************************************************/
CAPI void
SVector_Init(
    SVector *pVec, 
    size_t dElemSize
    )
{
    assert(pVec != NULL);
    pVec->dSize         = 0;
    pVec->dCapacity     = 0;
    pVec->dElemSize     = dElemSize;
    pVec->dIndex        = 0;
    pVec->pData         = (void*)0;
    pVec->pIterator     = (void*)0;
}

CAPI void 
SVector_Cleanup(SVector* pVec)
{
    assert(pVec != NULL);

    if (pVec->pData)
    {
        free(pVec->pData);
    }
    
    pVec->dSize     = 0;
    pVec->dCapacity = 0;
    pVec->dElemSize = 0;
    pVec->dIndex    = 0;
    pVec->dLine     = 0;
    pVec->pData     = NULL;
    pVec->pIterator = NULL;
    pVec            = NULL;
}

CAPI SVector 
SVector_Copy(const SVector* pVec) 
{
    SVector newVec;
    SVector_Init(
        &newVec, 
        pVec->dElemSize
    );

    if (pVec->dCapacity > 0) 
    {
        newVec.pData = malloc(pVec->dCapacity * pVec->dElemSize);

        if (newVec.pData == NULL) 
        {
            SVector_Cleanup(&newVec);
            return newVec;
        }
        newVec.dCapacity = pVec->dCapacity;
    }

    memcpy(newVec.pData, pVec->pData, pVec->dSize * pVec->dElemSize);
    newVec.dSize = pVec->dSize;

    return newVec;
}

CAPI int 
SVector_Reserve(SVector* pVec, size_t newCapacity) 
{
    void* pNewData = NULL;

    assert(pVec != NULL);

    if (newCapacity <= pVec->dCapacity) 
    {
        return 0;
    }

    pNewData = realloc(pVec->pData, newCapacity * pVec->dElemSize );

    if (!pNewData) 
    {
        return -1;
    }

    /* Zeroing for newly allocated space */
    memset((char*)pNewData + pVec->dCapacity * pVec->dElemSize, 0, 
           (newCapacity - pVec->dCapacity) * pVec->dElemSize);

    pVec->pData     = pNewData;
    pVec->dCapacity = newCapacity;

    return 0;
}

CAPI void
SVector_Resize(SVector* pVec, size_t dNewCapacity)
{
    /* @todo error handle for realloc + realloc must be S_Realloc */
    void* pNewData;
    
    assert(pVec != NULL);
    pNewData = realloc(pVec->pData, dNewCapacity * pVec->dElemSize);
    
    if (pNewData) 
    {
        if (dNewCapacity > pVec->dCapacity) 
        {
            /* zeroing newly allocated space */
            memset((char*)pNewData + pVec->dCapacity * pVec->dElemSize, 0, 
                   (dNewCapacity - pVec->dCapacity) * pVec->dElemSize);
        }

        pVec->dCapacity  = dNewCapacity;
        pVec->dIndex     = 0;
        pVec->pData      = pNewData;
        pVec->pIterator  = (void*)0;
    }
}

CAPI void 
SVector_Clear(SVector* pVec)
{
    assert(pVec != NULL);
    pVec->dSize = 0;
    pVec->dIndex = 0;
    pVec->pIterator = NULL;
}

CAPI void 
SVector_PushBack(SVector* pVec, const void* pValue)
{
    assert(pVec != NULL);
    if (pVec->dSize == pVec->dCapacity) 
    {
        /* Resize 2x */
        size_t dNewCapacity = pVec->dCapacity == 0 ? 1 : pVec->dCapacity << 1;
        SVector_Resize(pVec, dNewCapacity);
    }
    memcpy((char*)pVec->pData + pVec->dSize * pVec->dElemSize, pValue, pVec->dElemSize);
    pVec->dSize++;
}

CAPI int 
SVector_PopBack(SVector* pVec) 
{
    assert(pVec != NULL);

    if (pVec->dSize == 0) 
    {
        return -1;
    }

    pVec->dSize--; 
    return 0;
}

CAPI int 
SVector_Insert(SVector* pVec, size_t dIndex, const void* pValue) 
{
    assert(pVec != NULL);

    if (dIndex > pVec->dSize) 
    { 
        return -1;
    }

    if (pVec->dSize == pVec->dCapacity) 
    {
        SVector_Resize(pVec, pVec->dCapacity == 0 ? 1 : pVec->dCapacity * 2);
        /* int res = SVector_Resize(pVec, pVec->dCapacity == 0 ? 1 : pVec->dCapacity * 2);
        if (res != 0) return res; */
    }

    /* Shift elements to the right */
    memmove((char*)pVec->pData + (dIndex + 1) * pVec->dElemSize,
            (char*)pVec->pData + dIndex * pVec->dElemSize,
            (pVec->dSize - dIndex) * pVec->dElemSize);

    /* Insert the new element */
    memcpy((char*)pVec->pData + dIndex * pVec->dElemSize, &pValue, pVec->dElemSize);
    pVec->dSize++;

    return 0;
}

CAPI int 
SVector_Erase(SVector* pVec, size_t dIndex) 
{
    assert(pVec != NULL);

    if (dIndex >= pVec->dSize) 
    {
        return -1;
    }

    /* Shift elements to the left */
    memmove((char*)pVec->pData + dIndex * pVec->dElemSize,
            (char*)pVec->pData + (dIndex + 1) * pVec->dElemSize,
            (pVec->dSize - dIndex - 1) * pVec->dElemSize);

    pVec->dSize--;

    return 0;
}

CAPI void 
SVector_Set(SVector* pVec, size_t dIndex, const void* pValue) 
{
    assert(pVec != NULL);

    if (dIndex < pVec->dSize) 
    {
        memcpy((char*)pVec->pData + dIndex * pVec->dElemSize, pValue, pVec->dElemSize);
    } 
    else 
    {
        /* @todo handle out-of-bounds error */
        /* fprintf(stderr, "Index out of bounds: %zu (size: %zu)\n", dIndex, pVec->dSize); */
    }
}

CAPI void*
SVector_Get(SVector* pVec, size_t dIndex)
{
    assert(pVec != NULL);

    assert(dIndex < pVec->dCapacity);

    return (void*)((char*)pVec->pData + dIndex * pVec->dElemSize);
}

CAPI size_t
SVector_GetSize(const SVector* pVec)
{
    assert(pVec != NULL);

    return pVec->dSize;
}

CAPI long int
SVector_FindByValue(const SVector* pVec, const void* pValue)
{
    size_t i;

    assert(pVec != NULL);

    for (i = 0; i < pVec->dSize; i++) 
    {
        void* pElem = (char*)pVec->pData + i * pVec->dElemSize;
        if (memcmp(pElem, pValue, pVec->dElemSize) == 0) 
        {
            return (long int)i; 
        }
    }
    return -1;
}

CAPI long int
SVector_FindByPredicate(const SVector* pVec, SVectorPredicate pred, const void* context)
{
    /* Example: */
    /* 
    int value_equals_predicate(const void* element, void* context) 
    {
        return S_memcmp(element, context, sizeof(element)) == 0;
    } 
    SVector_FindByPredicate(&vec, value_equals_predicate, (void*)value);

    OR

    typedef struct 
    {
        int id;
        float value;
    } Data;
    int data_id_equals_predicate(const void* element, void* context) 
    {
        const Data* dataElement = (const Data*)element;
        int targetId = *(int*)context;
        return dataElement->id == targetId;
    }
    int targetId = 42;
    SVector_FindByPredicate(&vec, data_id_equals_predicate, &targetId);

    */

    size_t i;

    assert(pVec != NULL);

    for (i = 0; i < pVec->dSize; i++) 
    {
        void* pElem = (char*)pVec->pData + i * pVec->dElemSize;
        if (pred(pElem, context)) 
        {
            return (long int)i;
        }
    }
    return -1;
}
