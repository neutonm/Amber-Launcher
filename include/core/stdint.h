#ifndef SSTDINT_H_
#define SSTDINT_H_

#if defined(_MSC_VER)

typedef signed __int8       int8;
typedef unsigned __int8     uint8;

typedef signed __int16      int16;
typedef unsigned __int16    uint16;

typedef signed __int32      int32;
typedef unsigned __int32    uint32;

typedef signed __int64      int64;
typedef unsigned __int64    uint64;

#elif (SYSTEM_ARCH_BITNESS == 64)

typedef signed char         int8;
typedef unsigned char       uint8;

typedef signed short        int16;
typedef unsigned short      uint16;

typedef signed int          int32;
typedef unsigned int        uint32;

typedef signed long         int64;
typedef unsigned long       uint64;

typedef signed long         intptr;
typedef unsigned long       uintptr;

/* #define INTPTR_MIN          (-9223372036854775807LL - 1)
#define INTPTR_MAX          9223372036854775807LL
#define UINTPTR_MAX         18446744073709551615ULL */

#elif (SYSTEM_ARCH_BITNESS == 32)

typedef signed char         int8;
typedef unsigned char       uint8;

typedef signed short        int16;
typedef unsigned short      uint16;

typedef signed int          int32;
typedef unsigned int        uint32;

typedef signed long         int64;
typedef unsigned long       uint64;

typedef signed int          intptr;
typedef unsigned int        uintptr;

/* #define INTPTR_MIN          (-2147483647 - 1)
#define INTPTR_MAX          2147483647
#define UINTPTR_MAX         4294967295U */

#elif (SYSTEM_ARCH_BITNESS == 16)

typedef signed char         int8;
typedef unsigned char       uint8;

typedef signed short        int16;
typedef unsigned short      uint16;

/* long is 32-bit on many 16-bit systems */
typedef signed long         int32;  
typedef unsigned long       uint32;

/* 16-bit platforms may not have 64-bit support */
#if defined(__WATCOMC__)
typedef signed __int64      int64;
typedef unsigned __int64    uint64;
#else
/* MSDOS: Watcom & DJGPP supports long long */
typedef signed long long    int64_t;
typedef unsigned long long  uint64_t;
#endif

typedef signed short        intptr;
typedef unsigned short      uintptr;

/* #define INTPTR_MIN          (-32768)
#define INTPTR_MAX          32767
#define UINTPTR_MAX         65535
 */
#else
    #if 0
        error_label: "Unsupported architecture bitness!"
    #endif
#endif


/* Limits */
/* #define INT8_MAX   127
#define INT8_MIN   (-INT8_MAX - 1)
#define UINT8_MAX  255U

#define INT16_MAX  32767
#define INT16_MIN  (-INT16_MAX - 1)
#define UINT16_MAX 65535U

#define INT32_MAX  2147483647
#define INT32_MIN  (-INT32_MAX - 1)
#define UINT32_MAX 4294967295U

#ifdef ARCH_BIT_64
#define INT64_MAX  9223372036854775807L
#define INT64_MIN  (-INT64_MAX - 1)
#define UINT64_MAX 18446744073709551615UL
#endif */

#endif /* TYPES_H */
