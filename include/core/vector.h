#ifndef SVECTOR_H_
#define SVECTOR_H_

#include <stddef.h>
#include <core/apidef.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * MACROS
 ******************************************************************************/

/* In-case vector.h in used separately */
#ifndef CAPI
    #define CAPI
#endif

#define SVECTOR_DECLARE(NAME, TYPE) \
    SVector NAME

/******************************************************************************
 * STRUCTS
 ******************************************************************************/

typedef struct SVector
{
    size_t      dSize;
    size_t      dCapacity;
    size_t      dElemSize;
    size_t      dIndex;
    size_t      dLine;
    void*       pData;
    void*       pIterator;
    const char* sType;
    const char* sName;
    const char* sFile;
} SVector;

typedef int (*SVectorPredicate)(const void* pElement, const void* pContext);

/******************************************************************************
 * HEADER FUNCTION DECLARATIONS
 ******************************************************************************/

/**
 * @relatedalso SVector
 * @brief       Initialize vector
 * 
 * @param       pVec        SVector
 * @param       dElemSize   Element Size
 * @param       sType       Type (for debugging)
 * @param       sName       Name (for debugging)
 * @param       sFile       Source file (for debugging)
 * @param       dLine       Source file line (for debugging)
 */
extern CAPI void
SVector_Init(
    SVector *pVec, 
    size_t dElemSize);

/**
 * @relatedalso SVector
 * @brief       Frees dynamic array and sets values to zero
 * 
 * @param       pVec 
 */
extern CAPI void 
SVector_Cleanup(SVector* pVec);

/**
 * @relatedalso SVector
 * @brief       Creates a deep copy of the specified vector.
 * 
 * @param       pVec
 * @return      Returns a new vector that is a duplicate of the source vector.
 */
extern CAPI SVector 
SVector_Copy(const SVector* pVec);

/**
 * @relatedalso SVector
 * @brief       Reserves memory for at least the specified number of elements without changing the size of the vector.
 * 
 * @param       pVec 
 * @param       newCapacity 
 * @return      Returns 0 on success, or a negative value if an error occurs.
 */
extern CAPI int 
SVector_Reserve(SVector* pVec, size_t newCapacity);

/**
 * @relatedalso SVector
 * @brief       Reallocates vector based on new capacity size
 * 
 * @param       pVec 
 * @param       dNewCapacity 
 */
extern CAPI void
SVector_Resize(SVector* pVec, size_t dNewCapacity);

/**
 * @relatedalso SVector
 * @brief       Clears all elements from the vector, setting the size to 0 but keeping the memory allocated.
 * 
 * @param       pVec
 */
extern CAPI void 
SVector_Clear(SVector* pVec);

/**
 * @relatedalso SVector
 * @brief       Appends (copies) the given element value to the end of the container.
 * 
 * @param       pVec 
 * @param       pValue 
 */
extern CAPI void 
SVector_PushBack(SVector* pVec, const void* pValue);

/**
 * @relatedalso SVector
 * @brief       Removes the last element from the vector, reducing its size by one.
 * 
 * @param       pVec
 * @return      Returns 0 on success, or a negative value if the vector is empty.
 */
extern CAPI int 
SVector_PopBack(SVector* pVec);

/**
 * @relatedalso SVector
 * @brief       Inserts an element at the specified position, shifting subsequent elements.
 * 
 * @note        Uses O(n) due to shifting
 * 
 * @param       pVec 
 * @param       dIndex      Position at which to insert the new element.
 * @param       pValue      Pointer to the value to be inserted.
 * 
 * @return      Returns 0 on success, or a negative value if an error occurs (e.g., out of memory).
 */
extern CAPI int 
SVector_Insert(SVector* pVec, size_t dIndex, const void* pValue);

/**
 * @relatedalso SVector
 * @brief       Erases an element at the specified position, shifting subsequent elements down.
 * 
 * @note        Uses O(n) due to shifting
 * 
 * @param       pVec        
 * @param       dIndex      Position of the element to erase.
 * 
 * @return      Returns 0 on success, or a negative value if an error occurs (e.g., position out of bounds).
 */
extern CAPI int 
SVector_Erase(SVector* pVec, size_t dIndex);

/**
 * @relatedalso SVector
 * @brief       Sets element at given index to new value
 * 
 * @param       pVec 
 * @param       pIndex 
 * @param       pValue 
 */
extern CAPI void 
SVector_Set(SVector* pVec, size_t dIndex, const void* pValue);

/**
 * @relatedalso SVector
 * @brief       Retrieves value from the vector
 * @note        Not safe: doesn't check VEC
 * 
 * @param       pVec    SVector
 * @param       dIndex  Array Index
 * @param       TYPE    C Type
 */
extern CAPI void*
SVector_Get(SVector* pVec, size_t dIndex);
/* #define SVector_Get(pVec, dIndex, TYPE) (*((TYPE*)((char*)(pVec)->pData + (dIndex) * (pVec)->dElemSize))) */

/**
 * @brief       Checks vector size and returns true on > 0
 * 
 * @param       pVec 
 * @return      CBOOL 
 */
extern CAPI size_t
SVector_GetSize(const SVector* pVec);

/**
 * @relatedalso SVector
 * @brief       Loops through each element of the vector
 * @note        Not safe: doesn't check pVec
 * 
 * @param       pVec SVector
 */
#define SVector_ForEach(pVec)                                               \
    for ((pVec)->dIndex = 0;                                                \
         (pVec)->pData != NULL && (pVec)->dIndex < (pVec)->dSize;           \
         ++(pVec)->dIndex)                                                  \
        if (((pVec)->pIterator = (void*)((char*)(pVec)->pData +             \
                                         (pVec)->dIndex * (pVec)->dElemSize)), 1)

/**
 * @relatedalso SVector
 * @brief       Loops through each element of the vector with specified inded and iterator
 * @note        Not safe: doesn't check pVec
 * 
 * @param       pVec        SVector
 * @param       dIndex      Array Index
 * @param       pIterator   Custom iterator
 */
#define SVector_ForEachArgs(pVec, dIndex, pIterator) \
    for ( ;\
         (pVec)->pData != NULL && dIndex < (pVec)->dSize && \
         ((pIterator = (void*)((char*)(pVec)->pData + dIndex * (pVec)->dElemSize))); \
         ++dIndex)

/**
 * @relatedalso SVector
 * @brief       Initializes iterator for specified vector. 
 *              Use SVECTOR_ITERATOR for iterator operations.
 * @note        Can be used inside SVector_ForEach only
 * 
 * @param       TYPE type of object stored inside Svector
 * @param       pVec SVector
 */
#define SVector_InitIterator(TYPE, pVec) \
    TYPE* SVECTOR_ITERATOR = (TYPE*) (pVec)->pIterator; \
    if (SVECTOR_ITERATOR == ((void*)0)) { \
        continue; \
    }

/**
 * @relatedalso SVector
 * @brief       Initializes const iterator for specified vector. 
 *              Use SVECTOR_ITERATOR for iterator operations.
 * @note        Can be used inside SVector_ForEach only
 * 
 * @param       TYPE type of object stored inside Svector
 * @param       pVec SVector
 */
#define SVector_InitConstIterator(TYPE, pVec) \
    const TYPE* SVECTOR_ITERATOR = (TYPE*) (pVec)->pIterator; \
    if (SVECTOR_ITERATOR == ((void*)0)) { \
        continue; \
    }

/**
 * @relatedalso SVector
 * @brief       Returns index if one of vector's elements equals to given value using O(n)
 *              Returns -1 on failure.
 * 
 * @param       pVec 
 * @param       pValue 
 * @return      long int 
 */
extern CAPI long int 
SVector_FindByValue(const SVector* pVec, const void* pValue);

/**
 * @relatedalso SVector
 * @brief       Returns index of search result of an element where predicate returns value equal to context
 * 
 * @param       pVec 
 * @param       cbPred 
 * @param       pContext 
 * @return      long int 
 */
extern CAPI long int
SVector_FindByPredicate(const SVector* pVec, SVectorPredicate cbPred, const void* pContext);

#ifdef __cplusplus
}
#endif

#endif
