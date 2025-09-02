#ifndef __COMMON_H
#define __COMMON_H

#include <stddef.h>

#include <core/apidef.h>
#include <core/stdint.h>

/******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* Bool for ANSI C / C89 */
typedef enum {CFALSE, CTRUE = !CFALSE} cbool;
#define CBOOL cbool

/* POSIX required definition */
#define _XOPEN_SOURCE 700

/******************************************************************************
 * VARIANT / TAGGED UNION PATTERN
 ******************************************************************************/

/**
 * @brief   Enumeration of C types, sorted by size
 * 
 */
typedef enum 
{
    /* trivial */
    CTYPE_NULL,
    CTYPE_CHAR,
    CTYPE_UNSIGNED_CHAR,
    CTYPE_CONST_CHAR,
    CTYPE_SHORT,                
    CTYPE_UNSIGNED_SHORT,
    CTYPE_BOOL,
    CTYPE_INT,
    CTYPE_UNSIGNED_INT,
    CTYPE_LONG,
    CTYPE_UNSIGNED_LONG,
    CTYPE_FLOAT,
    CTYPE_LONG_LONG,
    CTYPE_UNSIGNED_LONG_LONG,
    CTYPE_VOID,
    CTYPE_DOUBLE,
    CTYPE_LONG_DOUBLE,
    /* non-trivial */
    CTYPE_LUAREF,
    CTYPE_LUATABLE,
    /* ...  */
    CTYPE_MAX
} ECType;

typedef union UVarData
{
    char                    _char;              /* CTYPE_CHAR               */
    unsigned char           _uchar;             /* CTYPE_UNSIGNED_CHAR      */
    const char*             _constchar;         /* CTYPE_CONST_CHAR         */
    short                   _short;             /* CTYPE_SHORT              */
    unsigned short          _ushort;            /* CTYPE_UNSIGNED_SHORT     */
    CBOOL                   _bool;              /* CTYPE_BOOL               */
    int                     _int;               /* CTYPE_INT                */
    unsigned int            _uint;              /* CTYPE_UNSIGNED_INT       */
    long                    _long;              /* CTYPE_LONG               */
    unsigned long           _ulong;             /* CTYPE_UNSIGNED_LONG      */
    float                   _float;             /* CTYPE_FLOAT              */
    long long               _longlong;          /* CTYPE_LONG_LONG          */
    unsigned long long      _ulonglong;         /* CTYPE_UNSIGNED_LONG_LONG */
    void*                   _void;              /* CTYPE_VOID               */
    double                  _double;            /* CTYPE_DOUBLE             */
    long double             _longdouble;        /* CTYPE_LONG_DOUBLE        */
} UVarData;

typedef struct SVar
{
    ECType      eType;
    uint32      dFlags;
    size_t      dSize;
    UVarData    uData;
} SVar;

typedef struct SVarTableEntry {
    SVar tKey;
    SVar tValue;
} SVarTableEntry;

typedef struct SVarTable {
    size_t           dCount;
    SVarTableEntry  *pEntries;
    int              dLuaRef;
} SVarTable;

#define SVAR_NULL(v)                                \
    do {                                            \
        (v).eType       = CTYPE_NULL;               \
        (v).dFlags      = 0;                        \
        (v).dSize       = sizeof((v).uData._char);  \
        (v).uData._char = '\0';                     \
    } while (0)

#define SVAR_BOOL(v,x)                              \
    do {                                            \
        (v).eType = CTYPE_BOOL;                     \
        (v).dFlags = 0;                             \
        (v).dSize = sizeof((v).uData._bool);        \
        (v).uData._bool = (x);                      \
    } while (0)

#define SVAR_CHAR(v,x)                              \
    do {                                            \
        (v).eType = CTYPE_CHAR;                     \
        (v).dFlags = 0;                             \
        (v).dSize = sizeof((v).uData._char);        \
        (v).uData._char = (x);                      \
    } while (0)

#define SVAR_UCHAR(v,x)                             \
    do {                                            \
        (v).eType = CTYPE_UNSIGNED_CHAR;            \
        (v).dFlags = 0;                             \
        (v).dSize = sizeof((v).uData._uchar);       \
        (v).uData._uchar = (x);                     \
    } while (0)

#define SVAR_CONSTCHAR(v,x)                         \
    do {                                            \
        (v).eType = CTYPE_CONST_CHAR;               \
        (v).dFlags = 0;                             \
        (v).dSize = sizeof((v).uData._constchar);   \
        (v).uData._constchar = (x);                 \
    } while (0)

#define SVAR_SHORT(v,x)                             \
    do {                                            \
        (v).eType = CTYPE_SHORT;                    \
        (v).dFlags = 0;                             \
        (v).dSize = sizeof((v).uData._short);       \
        (v).uData._short = (x);                     \
    } while (0)

#define SVAR_USHORT(v,x)                            \
    do {                                            \
        (v).eType = CTYPE_UNSIGNED_SHORT;           \
        (v).dFlags = 0;                             \
        (v).dSize = sizeof((v).uData._ushort);      \
        (v).uData._ushort = (x);                    \
    } while (0)

#define SVAR_INT(v,x)                               \
    do {                                            \
        (v).eType = CTYPE_INT;                      \
        (v).dFlags = 0;                             \
        (v).dSize = sizeof((v).uData._int);         \
        (v).uData._int = (x);                       \
    } while (0)

#define SVAR_UINT(v,x)                              \
    do {                                            \
        (v).eType = CTYPE_UNSIGNED_INT;             \
        (v).dFlags = 0;                             \
        (v).dSize = sizeof((v).uData._uint);        \
        (v).uData._uint = (x);                      \
    } while (0)

#define SVAR_LONG(v,x)                              \
    do {                                            \
        (v).eType = CTYPE_LONG;                     \
        (v).dFlags = 0;                             \
        (v).dSize = sizeof((v).uData._long);        \
        (v).uData._long = (x);                      \
    } while (0)

#define SVAR_ULONG(v,x)                             \
    do {                                            \
        (v).eType = CTYPE_UNSIGNED_LONG;            \
        (v).dFlags = 0;                             \
        (v).dSize = sizeof((v).uData._ulong);       \
        (v).uData._ulong = (x);                     \
    } while (0)

#define SVAR_FLOAT(v,x)                             \
    do {                                            \
        (v).eType = CTYPE_FLOAT;                    \
        (v).dFlags = 0;                             \
        (v).dSize = sizeof((v).uData._float);       \
        (v).uData._float = (x);                     \
    } while (0)

#define SVAR_LLONG(v,x)                             \
    do {                                            \
        (v).eType = CTYPE_LONG_LONG;                \
        (v).dFlags = 0;                             \
        (v).dSize = sizeof((v).uData._longlong);    \
        (v).uData._longlong = (x);                  \
    } while (0)

#define SVAR_ULLONG(v,x)                            \
    do {                                            \
        (v).eType = CTYPE_UNSIGNED_LONG_LONG;       \
        (v).dFlags = 0;                             \
        (v).dSize = sizeof((v).uData._ulonglong);   \
        (v).uData._ulonglong = (x);                 \
    } while (0)

/* void types are exceptional */
#define SVAR_VOID(v, ptr, sz)                       \
    do {                                            \
        (v).eType         = CTYPE_VOID;             \
        (v).dFlags        = 0;                      \
        (v).dSize         = (sz);                   \
        (v).uData._void   = (ptr);                  \
    } while (0)

#define SVAR_DOUBLE(v,x)                            \
    do {                                            \
        (v).eType = CTYPE_DOUBLE;                   \
        (v).dFlags = 0;                             \
        (v).dSize = sizeof((v).uData._double);      \
        (v).uData._double = (x);                    \
    } while (0)

#define SVAR_LDOUBLE(v,x)                           \
    do {                                            \
        (v).eType = CTYPE_LONG_DOUBLE;              \
        (v).dFlags = 0;                             \
        (v).dSize = sizeof((v).uData._longdouble);  \
        (v).uData._longdouble = (x);                \
    } while (0)

/**
 * non-trivial: lua reference as int
 * @note include lua headers before usage
 */
#define SVAR_LUAREF(v, L, id)                               \
    do {                                                    \
        (v).eType = CTYPE_LUAREF;                           \
        (v).dFlags = 0;                                     \
        lua_pushvalue((L), (id));                           \
        (v).dSize = sizeof((v).uData._int);                 \
        (v).uData._int = luaL_ref((L), LUA_REGISTRYINDEX);  \
    } while (0)

#define SVAR_LUATABLE(v, ptr)                    \
    do {                                            \
        (v).eType       = CTYPE_LUATABLE;           \
        (v).dFlags      = 0;                        \
        (v).dSize       = sizeof(void*);            \
        (v).uData._void = (ptr);                \
    } while (0)

/* Fixed‑width aliases */
#define SVAR_INT8(v,x)              SVAR_CHAR((v),(x))
#define SVAR_UINT8(v,x)             SVAR_UCHAR((v),(x))
#define SVAR_INT16(v,x)             SVAR_SHORT((v),(x))
#define SVAR_UINT16(v,x)            SVAR_USHORT((v),(x))
#define SVAR_INT32(v,x)             SVAR_INT((v),(x))
#define SVAR_UINT32(v,x)            SVAR_UINT((v),(x))
#define SVAR_INT64(v,x)             SVAR_LLONG((v),(x))
#define SVAR_UINT64(v,x)            SVAR_ULLONG((v),(x))

/* Getters */
#define SVAR_GET_BOOL(v)            ((v).uData._bool)
#define SVAR_GET_CHAR(v)            ((v).uData._char)
#define SVAR_GET_UCHAR(v)           ((v).uData._uchar)
#define SVAR_GET_CONSTCHAR(v)       ((v).uData._constchar)
#define SVAR_GET_SHORT(v)           ((v).uData._short)
#define SVAR_GET_USHORT(v)          ((v).uData._ushort)
#define SVAR_GET_INT(v)             ((v).uData._int)
#define SVAR_GET_UINT(v)            ((v).uData._uint)
#define SVAR_GET_LONG(v)            ((v).uData._long)
#define SVAR_GET_ULONG(v)           ((v).uData._ulong)
#define SVAR_GET_LLONG(v)           ((v).uData._longlong)
#define SVAR_GET_ULLONG(v)          ((v).uData._ulonglong)
#define SVAR_GET_FLOAT(v)           ((v).uData._float)
#define SVAR_GET_DOUBLE(v)          ((v).uData._double)
#define SVAR_GET_LDOUBLE(v)         ((v).uData._longdouble)
#define SVAR_GET_VOID(v)            ((v).uData._void)
#define SVAR_GET_VOID_SIZE(v)       ((v).uData.dSize)

/* Non-trivial getters */
#define SVAR_GET_LUAREF(v)          ((v).uData._int)
#define SVAR_GET_LUATABLE(v)        ((v).uData._void)

/* Fixed-width getter aliases */
#define SVAR_GET_INT8(v)            SVAR_GET_CHAR(v)
#define SVAR_GET_UINT8(v)           SVAR_GET_UCHAR(v)
#define SVAR_GET_INT16(v)           SVAR_GET_SHORT(v)
#define SVAR_GET_UINT16(v)          SVAR_GET_USHORT(v)
#define SVAR_GET_INT32(v)           SVAR_GET_INT(v)
#define SVAR_GET_UINT32(v)          SVAR_GET_UINT(v)
#define SVAR_GET_INT64(v)           SVAR_GET_LLONG(v)
#define SVAR_GET_UINT64(v)          SVAR_GET_ULLONG(v)

/* Individual type checks */
#define SVAR_TYPE(v)              ((v).eType)
#define SVAR_IS_NULL(v)           (SVAR_TYPE(v) == CTYPE_NULL)
#define SVAR_IS_CHAR(v)           (SVAR_TYPE(v) == CTYPE_CHAR)
#define SVAR_IS_UCHAR(v)          (SVAR_TYPE(v) == CTYPE_UNSIGNED_CHAR)
#define SVAR_IS_CONSTCHAR(v)      (SVAR_TYPE(v) == CTYPE_CONST_CHAR)
#define SVAR_IS_SHORT(v)          (SVAR_TYPE(v) == CTYPE_SHORT)
#define SVAR_IS_USHORT(v)         (SVAR_TYPE(v) == CTYPE_UNSIGNED_SHORT)
#define SVAR_IS_BOOL(v)           (SVAR_TYPE(v) == CTYPE_BOOL)
#define SVAR_IS_INT(v)            (SVAR_TYPE(v) == CTYPE_INT)
#define SVAR_IS_UINT(v)           (SVAR_TYPE(v) == CTYPE_UNSIGNED_INT)
#define SVAR_IS_LONG(v)           (SVAR_TYPE(v) == CTYPE_LONG)
#define SVAR_IS_ULONG(v)          (SVAR_TYPE(v) == CTYPE_UNSIGNED_LONG)
#define SVAR_IS_LLONG(v)          (SVAR_TYPE(v) == CTYPE_LONG_LONG)
#define SVAR_IS_ULLONG(v)         (SVAR_TYPE(v) == CTYPE_UNSIGNED_LONG_LONG)
#define SVAR_IS_FLOAT(v)          (SVAR_TYPE(v) == CTYPE_FLOAT)
#define SVAR_IS_DOUBLE(v)         (SVAR_TYPE(v) == CTYPE_DOUBLE)
#define SVAR_IS_LDOUBLE(v)        (SVAR_TYPE(v) == CTYPE_LONG_DOUBLE)
#define SVAR_IS_VOID(v)           (SVAR_TYPE(v) == CTYPE_VOID)
#define SVAR_IS_LUAREF(v)         (SVAR_TYPE(v) == CTYPE_LUAREF)
#define SVAR_IS_LUATABLE(v)       (SVAR_TYPE(v) == CTYPE_LUATABLE)

/* Fixed-width aliases check */
#define SVAR_IS_INT8(v)           SVAR_IS_CHAR(v)
#define SVAR_IS_UINT8(v)          SVAR_IS_UCHAR(v)
#define SVAR_IS_INT16(v)          SVAR_IS_SHORT(v)
#define SVAR_IS_UINT16(v)         SVAR_IS_USHORT(v)
#define SVAR_IS_INT32(v)          SVAR_IS_INT(v)
#define SVAR_IS_UINT32(v)         SVAR_IS_UINT(v)
#define SVAR_IS_INT64(v)          SVAR_IS_LLONG(v)
#define SVAR_IS_UINT64(v)         SVAR_IS_ULLONG(v)

/* SVar Key */
#define SVAR_KEYBUNDLE_MAX        32

typedef struct SVarKey
{
    const char      *sKey;
    SVar            tVar;
} SVarKey;

typedef struct SVarKeyBundle
{
    size_t  dCount;
    SVarKey tKeys[SVAR_KEYBUNDLE_MAX];
} SVarKeyBundle;

#define SVARKEYB_INIT(b)   do { (b).dCount = 0; } while (0)
#define SVARKEYB_PUSH(bundle, keyname, SVAR_SET_MACRO, val)                 \
    do {                                                                    \
        assert((bundle).dCount < SVAR_KEYBUNDLE_MAX);                       \
        SVAR_SET_MACRO((bundle).tKeys[(bundle).dCount].tVar, (val));        \
        (bundle).tKeys[(bundle).dCount].sKey = (keyname);                   \
        ++(bundle).dCount;                                                  \
    } while (0)

#define SVARKEYB_NULL(b,k)                                                  \
    do {                                                                    \
        assert((b).dCount < SVAR_KEYBUNDLE_MAX);                            \
        SVAR_NULL((b).tKeys[(b).dCount].tVar);                              \
        (b).tKeys[(b).dCount].sKey = (k);                                   \
        ++(b).dCount;                                                       \
    } while (0)

#define SVARKEYB_CHAR(b,k,v)     SVARKEYB_PUSH(b, k, SVAR_CHAR,            v)
#define SVARKEYB_UCHAR(b,k,v)    SVARKEYB_PUSH(b, k, SVAR_UCHAR,           v)
#define SVARKEYB_CONSTCHAR(b,k,v)SVARKEYB_PUSH(b, k, SVAR_CONSTCHAR,       v)
#define SVARKEYB_SHORT(b,k,v)    SVARKEYB_PUSH(b, k, SVAR_SHORT,           v)
#define SVARKEYB_USHORT(b,k,v)   SVARKEYB_PUSH(b, k, SVAR_USHORT,          v)
#define SVARKEYB_BOOL(b,k,v)     SVARKEYB_PUSH(b, k, SVAR_BOOL,            v)
#define SVARKEYB_INT(b,k,v)      SVARKEYB_PUSH(b, k, SVAR_INT,             v)
#define SVARKEYB_UINT(b,k,v)     SVARKEYB_PUSH(b, k, SVAR_UINT,            v)
#define SVARKEYB_LONG(b,k,v)     SVARKEYB_PUSH(b, k, SVAR_LONG,            v)
#define SVARKEYB_ULONG(b,k,v)    SVARKEYB_PUSH(b, k, SVAR_ULONG,           v)
#define SVARKEYB_FLOAT(b,k,v)    SVARKEYB_PUSH(b, k, SVAR_FLOAT,           v)
#define SVARKEYB_LLONG(b,k,v)    SVARKEYB_PUSH(b, k, SVAR_LLONG,           v)
#define SVARKEYB_ULLONG(b,k,v)   SVARKEYB_PUSH(b, k, SVAR_ULLONG,          v)
#define SVARKEYB_DOUBLE(b,k,v)   SVARKEYB_PUSH(b, k, SVAR_DOUBLE,          v)
#define SVARKEYB_LDOUBLE(b,k,v)  SVARKEYB_PUSH(b, k, SVAR_LDOUBLE,         v)

#define SVARKEYB_VOID(b,k,ptr,sz)                                           \
    do {                                                                    \
        assert((b).dCount < SVAR_KEYBUNDLE_CAP);                            \
        SVAR_VOID((b).tKeys[(b).dCount].tVar, (ptr), (sz));                 \
        (b).tKeys[(b).dCount].sKey = (k);                                   \
        ++(b).dCount;                                                       \
    } while (0)

#define SVARKEYB_LUATABLE(b, key, ptr)                                      \
    SVARKEYB_PUSH(b, key, SVAR_LUATABLE, ptr)

/******************************************************************************
 * COMMON PREPROCESSOR FUNCTIONALITY
 ******************************************************************************/

/* Supressed unused variable compiler warning */
#define UNUSED(x) ((void)x)

/* General check for pointers */
#define IS_VALID(ptr) ((ptr) != NULL)

/* Bitmask flag operators */
#define FLAG_HAS(f,m)       (((f) &  (m)) != 0)
#define FLAG_HAS_ALL(f,m)   (((f) &  (m)) == (m))
#define FLAG_HAS_NONE(f,m)  (((f) &  (m)) == 0)

#define FLAG_SET(f,m)       ((f) |=  (m))
#define FLAG_CLEAR(f,m)     ((f) &= ~(m))
#define FLAG_TOGGLE(f,m)    ((f) ^=  (m))

#define FLAG_REPLACE(f,mask,value) \
    do { (f) = ((f) & ~(mask)) | ((value) & (mask)); } while (0)

/* Helpers for byte printing */
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  ((byte) & 0x80 ? '1' : '0'), \
  ((byte) & 0x40 ? '1' : '0'), \
  ((byte) & 0x20 ? '1' : '0'), \
  ((byte) & 0x10 ? '1' : '0'), \
  ((byte) & 0x08 ? '1' : '0'), \
  ((byte) & 0x04 ? '1' : '0'), \
  ((byte) & 0x02 ? '1' : '0'), \
  ((byte) & 0x01 ? '1' : '0')

#define U32_TO_BINARY_PATTERN  \
    BYTE_TO_BINARY_PATTERN BYTE_TO_BINARY_PATTERN \
    BYTE_TO_BINARY_PATTERN BYTE_TO_BINARY_PATTERN

#define U32_TO_BINARY(u32)            \
    BYTE_TO_BINARY((uint8_t)((u32) >> 24)), \
    BYTE_TO_BINARY((uint8_t)((u32) >> 16)), \
    BYTE_TO_BINARY((uint8_t)((u32) >>  8)), \
    BYTE_TO_BINARY((uint8_t)((u32)      ))

#ifdef __cplusplus
#define __EXTERN_C \
    extern "C" \
    {
#define __END_C }
#else
#define __EXTERN_C
#define __END_C
#endif

#endif
