/*
 *  This program implements the Secure Hashing Algorithm SHA512 as
 *  described in FIPS PUB 180-3 published in October 2008, see
 *
 *  http://csrc.nist.gov/publications/fips/fips180-3/fips180-3_final.pdf
 *
 *  Copyright (C) 2009 Jens Thoms Toerring <jt@toerring.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the program. If not, see <http://www.gnu.org/licenses/>.
 */

#if ! defined SHA_TYPES_HEADER_
#define SHA_TYPES_HEADER_

#include <limits.h>
#include <stdlib.h>


#if defined __STRICT_ANSI__
#define inline
#endif


#if ! defined SIZE_MAX
#define SIZE_MAX              ((size_t) -1)
#endif


#if UINT_MAX >= 0xFFFFFFFFUL
typedef unsigned int  sha_u32;
#if UINT_MAX == 0xFFFFFFFFUL
#define SHA_T32(x)            (x)
#else
#define SHA_T32(x)            ((x) & 0xFFFFFFFFUL)
#endif
#else
typedef unsigned long sha_u32;
#if ULONG_MAX == 0xFFFFFFFFUL
#define SHA_T32(x)            (x)
#else
#define SHA_T32(x)            ((x) & 0xFFFFFFFFUL)
#endif
#endif


#if CHAR_BIT == 8
#define SHA_T8(x)             (x)
#define SHA_T8L(x)            ((sha_u32) (x))
#else
#define SHA_T8(x)             (x & 0xFFU)
#define SHA_T8L(x)            ((sha_u32) (x & 0xFFU))
#endif


#if ULONG_MAX >> 32 >= 0xFFFFFFFFUL        /* 64-bit (and more) systems */

typedef unsigned long sha_u64;

#if ULONG_MAX >> 32 == 0xFFFFFFFFUL
#define SHA_T64(x)            (x)
#else
#define SHA_T64(x)            ((x) & 0xFFFFFFFFFFFFFFFFUL)
#endif

#define sha_u64_set(hi, lo)  (SHA_T64((sha_u64) (hi) << 32) | (lo))
#define sha_u64_hi(x)        ((x) >> 32)
#define sha_u64_low(x)       ((x) & 0xFFFFFFFFUL)
#define sha_u64_inv(x)       SHA_T64(~(x))
#define sha_u64_eq(x, y)     ((x) == (y))
#define sha_u64_lt(x, y)     ((x) < (y))
#define sha_u64_and(x, y)    ((x) & (y))
#define sha_u64_or(x, y)     ((x) | (y))
#define sha_u64_xor(x, y)    ((x) ^ (y))
#define sha_u64_plus(x, y)   SHA_T64((x) + (y))
#define sha_u64_shl(x, n)    SHA_T64((sha_u64) (x) << (n))
#define sha_u64_shr(x, n)    ((x) >> (n))


/*----------------------------------------------------------------*
 * Returns x + y or 0 value on overflow (where x is a sha_u64
 * while y is of type size_t
 *----------------------------------------------------------------*/

#if defined NEED_U64_SIZET_PLUS
static inline sha_u64
sha_u64_sizet_plus(sha_u64 x,
                   size_t  y)
{
    if (y >> 32 > 0xFFFFFFFFUL || SHA_T64(x + y) < x)
        return 0;
    return x + y;
}
#endif


#else                                        /* 32 bit etc. systems */

typedef struct {
    sha_u32 hi;
    sha_u32 lo;
} sha_u64;


/*----------------------------------------------------------------*
 * Returns sha_u64 value set up from two 32-bit values
 *----------------------------------------------------------------*/

#if defined NEED_U64_SET || defined NEED_U128_SET
static inline sha_u64
sha_u64_set(sha_u32 hi,
            sha_u32 lo)
{
    sha_u64 r;

    r.hi = SHA_T32(hi);
    r.lo = SHA_T32(lo);

    return r;
}
#endif


/*----------------------------------------------------------------*
 * Returns the upper 32-bit portion of a 64-bit variable
 *----------------------------------------------------------------*/

#if defined NEED_U64_HI
static inline sha_u32
sha_u64_hi(sha_u64 x)
{
    return x.hi;
}
#endif


/*----------------------------------------------------------------*
 * Returns the lower 32-bit portion of a 64-bit variable
 *----------------------------------------------------------------*/

#if defined NEED_U64_LOW || defined NEED_U128_LOW
static inline sha_u32
sha_u64_low(sha_u64 x)
{
    return x.lo;
}
#endif


/*----------------------------------------------------------------*
 * Returns the inverse of a sha_u64 value
 *----------------------------------------------------------------*/

#if defined NEED_U64_INV
static inline sha_u64
sha_u64_inv(sha_u64 x)
{
    sha_u64 r;

    r.hi = SHA_T32(~x.hi);
    r.lo = SHA_T32(~x.lo);

    return r;
}
#endif


#if defined NEED_U64_LT || defined NEED_U128_LT

/*----------------------------------------------------------------*
 * Returns x == y
 *----------------------------------------------------------------*/

static inline int
sha_u64_eq(sha_u64 x,
           sha_u64 y)
{
    return x.hi == y.hi && x.lo == y.lo;
}


/*----------------------------------------------------------------*
 * Returns x < y
 *----------------------------------------------------------------*/

static inline int
sha_u64_lt(sha_u64 x,
           sha_u64 y)
{
    return x.hi < y.hi || (x.hi == y.hi && x.lo < y.lo);
}

#endif


/*----------------------------------------------------------------*
 * Returns x & y
 *----------------------------------------------------------------*/

#if defined NEED_U64_AND
static inline sha_u64
sha_u64_and(sha_u64 x,
            sha_u64 y)
{
    sha_u64 r;

    r.hi = x.hi & y.hi;
    r.lo = x.lo & y.lo;

    return r;
}
#endif


/*----------------------------------------------------------------*
 * Returns x | y
 *----------------------------------------------------------------*/

#if defined NEED_U64_OR
static inline sha_u64
sha_u64_or(sha_u64 x,
           sha_u64 y)
{
    sha_u64 r;

    r.hi = x.hi | y.hi;
    r.lo = x.lo | y.lo;

    return r;
}
#endif


/*----------------------------------------------------------------*
 * Returns x ^ y
 *----------------------------------------------------------------*/

#if defined NEED_U64_XOR
static inline sha_u64
sha_u64_xor(sha_u64 x,
            sha_u64 y)
{
    sha_u64 r;

    r.hi = x.hi ^ y.hi;
    r.lo = x.lo ^ y.lo;

    return r;
}
#endif


/*----------------------------------------------------------------*
 * Returns x + y
 *----------------------------------------------------------------*/

#if defined NEED_U64_PLUS || defined NEED_U128_PLUS
static inline sha_u64
sha_u64_plus(sha_u64 x,
             sha_u64 y)
{
    sha_u64 r;

    r.lo = SHA_T32(x.lo + y.lo);
    r.hi = SHA_T32(x.hi + y.hi + (r.lo < x.lo));

    return r;
}
#endif


/*----------------------------------------------------------------*
 * Returns x + y or 0 value on overflow where x is a sha_u64
 *  while y is of type size_t
 *----------------------------------------------------------------*/

#if defined NEED_U64_SIZET_PLUS
static inline sha_u64
sha_u64_sizet_plus(sha_u64 x,
                   size_t  y)
{
    sha_u64 r;
    sha_u32 b[2];

    b[0] = (sha_u32)SHA_T32(y);
    y >>= 16;
    y >>= 16;
    b[1] = (sha_u32)SHA_T32(y);
    y >>= 16;
    y >>= 16;

    if (y > 0)
        return sha_u64_set(0, 0);

    r.lo = SHA_T32(x.lo + b[0]);
    r.hi = SHA_T32(x.hi + b[1] + (r.lo < x.lo));

    if (r.hi < x.hi || (r.hi == x.hi && r.lo < x.lo))
        return sha_u64_set(0, 0);

    return r;
}
#endif


/*----------------------------------------------------------------*
 * Return x << n
 *----------------------------------------------------------------*/

#if defined NEED_U64_SHL || defined NEED_U128_SHR
static inline sha_u64
sha_u64_shl(sha_u64 x,
            int     n)
{
    sha_u64 r;

    if (n < 32) {
        r.hi = SHA_T32((x.hi << n) | (x.lo >> (32 - n)));
        r.lo = SHA_T32(x.lo << n);
    } else {
        r.hi = SHA_T32(x.lo << (n - 32));
        r.lo = 0;
    }

    return r;
}
#endif


/*----------------------------------------------------------------*
 * Returns x >> n
 *----------------------------------------------------------------*/

#if defined NEED_U64_SHR || defined NEED_U128_SHR
static inline sha_u64
sha_u64_shr(sha_u64 x,
            int     n)
{
    sha_u64 r;

    if (n < 32) {
        r.hi = x.hi >> n;
        r.lo = SHA_T32((x.hi << (32 - n)) | (x.lo >> n));
    } else {
        r.hi = 0;
        r.lo = x.hi >> (n - 32);
    }

    return r;
}
#endif

#endif /* ! ULONG_MAX >= 0xFFFFFFFFFFFFFFFFUL */


#if ULONG_MAX >> 96 >= 0xFFFFFFFFUL

typedef unsigned long sha_u128;

#if ULONG_MAX >> 96 > 0xFFFFFFFFUL
#define T128(x)           ((x) & 0x0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFUL)
#else
#define T128(x)           (x)
#endif

#define sha_u128_set(b3, b2, b1, b0)                                     \
                                (  SHA_T128((sha_u128) (b3) << 96)  \
                                 | SHA_T128((sha_u128) (b2) << 64)  \
                                 | SHA_T128((sha_u128) (b1) << 32)  \
                                 | SHA_T128(b0))
#define sha_u128_lt(x, y)       ((x) < (y))
#define sha_u128_eq(x, y)       ((x) == (y))
#define sha_u128_plus(x, y)     T128((x) + (y))
#define sha_u128_low(x)         ((x) & 0xFFFFFFFFUL)
#define sha_u128_shr(x, n)      ((x) >> (n))


/*----------------------------------------------------------------*
 * Returns x + y or 0 value on overflow where x is a sha_u128
 *  while y is of type size_t
 *----------------------------------------------------------------*/

#if defined defined NEED_U128_SIZET_PLUS
static inline sha_u128
sha_u64_sizet_plus(sha_u128 x,
                   size_t   y)
{
    if (y >> 96 > 0xFFFFFFFFUL || SHA_T128(x + y) < x)
        return 0;
    return x + y;
}
#endif


#else

typedef struct {
    sha_u64 hi;
    sha_u64 lo;
} sha_u128;


/*----------------------------------------------------------------*
 * Returns sha_u128 value set up from four 32-bit values
 *----------------------------------------------------------------*/

#if defined NEED_U128_SET
static inline sha_u128
sha_u128_set(sha_u32 b3,
             sha_u32 b2,
             sha_u32 b1,
             sha_u32 b0)
{
    sha_u128 r;

    r.hi = sha_u64_set(b3, b2);
    r.lo = sha_u64_set(b1, b0);

    return r;
}
#endif


/*----------------------------------------------------------------*
 * Returns x < y
 *----------------------------------------------------------------*/

#if defined NEED_U128_LT
static inline int
sha_u128_lt(sha_u128 x,
            sha_u128 y)
{
    return    sha_u64_lt(x.hi, y.hi)
           || (sha_u64_eq(x.hi, y.hi) && sha_u64_lt(x.lo, y.lo));
}


/*----------------------------------------------------------------*
 * Returns x == y
 *----------------------------------------------------------------*/

static inline int
sha_u128_eq(sha_u128 x,
            sha_u128 y)
{
    return sha_u64_eq(x.hi, y.hi) && sha_u64_eq(x.lo, y.lo);
}
#endif


/*----------------------------------------------------------------*
 * Returns x + y
 *----------------------------------------------------------------*/

#if defined NEED_U128_PLUS
static inline sha_u128
sha_u128_plus(sha_u128 x,
              sha_u128 y)
{
    sha_u128 r;

    r.lo = sha_u64_plus(x.lo, y.lo);
    r.hi = sha_u64_plus(sha_u64_plus(x.hi, y.hi),
                        sha_u64_set(0, sha_u64_lt(r.lo, x.lo)));

    return r;
}
#endif


/*----------------------------------------------------------------*
 * Returns x + y or 0 value on overflow where x is a sha_u128
 *  while y is of type size_t
 *----------------------------------------------------------------*/

#if defined NEED_U128_SIZET_PLUS
static inline sha_u128
sha_u128_sizet_plus(sha_u128 x,
                    size_t   y)
{
    sha_u128 r;
    sha_u128 y128;
    sha_u32 b[4];

    /* Split the size_t value into 32-bit parts without triggering any
       warnings warnings due to shift that exceed the width of the variable */

    b[0] = SHA_T32(y);
    y >>= 16;
    y >>= 16;
    b[1] = SHA_T32(y);
    y >>= 16;
    y >>= 16;
    b[2] = SHA_T32(y);
    y >>= 16;
    y >>= 16;
    b[3] = SHA_T32(y);
    y >>= 16;
    y >>= 16;

    /* If a size_t has more than 128-bits and one of this above-128 bits
       is set we already got an overflow */

    if (y > 0)
        return sha_u128_set(0, 0, 0, 0);

    y128 = sha_u128_set(b[3], b[2], b[1], b[0]);

    r = sha_u128_plus(x, y128);

    /* Check for overflow */

    if (   sha_u64_lt(r.hi, x.hi)
        || (sha_u64_eq(r.hi, x.hi) && sha_u64_lt(r.lo, x.lo)))
        r = sha_u128_set(0, 0, 0, 0);

    return r;
}
#endif


/*----------------------------------------------------------------*
 * Returns the lower 32-bit portion of a 128-bit variable
 *----------------------------------------------------------------*/

#if defined NEED_U128_LOW
static inline sha_u32
sha_u128_low(sha_u128 x)
{
    return sha_u64_low(x.lo);
}
#endif


/*----------------------------------------------------------------*
 * Returns x >> n
 *----------------------------------------------------------------*/

#if defined NEED_U128_SHR
static inline sha_u128
sha_u128_shr(sha_u128 x,
             int      n)
{
    sha_u128 r;

    if (n < 64) {
        r.hi = sha_u64_shr(x.hi, n);
        r.lo = sha_u64_or(sha_u64_shl(x.hi, (64 - n)),
                          sha_u64_shr(x.lo, n));
    } else {
        r.hi = sha_u64_shr(x.hi, n - 64);
    }

    return r;
}
#endif

#endif /* ! ULONG_MAX >= 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFUL */


#endif /* ! defined SHA_TYPES_HEADER_ */
 

/*
 * Local variables:
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
