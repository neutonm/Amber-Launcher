#include <core/opsys.h>

#include <stdio.h>

/******************************************************************************
 * HEADER FUNCTION DEFINITIONS
 ******************************************************************************/

CAPI int 
AmberLauncher_GetApplicationBitness(void)
{
    return sizeof(void*) << 3;
}

CAPI CBOOL 
AmberLauncher_IsLittleEndian(void)
{
    union 
    {
        volatile uint32 i;
        volatile char c[4];
    } e = { 0x01000000 };

    return !e.c[0];
}
