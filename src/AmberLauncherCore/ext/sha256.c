/*
 *  This program implements the Secure Hashing Algorithm SHA256 as
 *  described in FIPS PUB 180-3 published in October 2008, see
 *
 *  http://csrc.nist.gov/publications/fips/fips180-3/fips180-3_final.pdf
 *
 *  PLEASE NOTE: While I implemented the SHA256 algorithm as good as my
 *               abilities and understanding of the standard cited above
 *               allow I'm not claiming that it is error free. Thus if
 *               you intend to use it e.g. in cryptographic applications
 *               you must review and test it carefully yourself!
 *
 *  A lot of ideas came from the example implementation from RFC 3174
 *  by D. Eastlake, 3rd (Motorola) and P. Jones (Cisco Systems), see
 *  e.g.
 *
 *  http://www.faqs.org/rfcs/rfc3174.html
 *
 *  The part for dealing with 64-bit numbers on systems that lack such
 *  a type has directly been taken from code written by Paul Eggert,
 *  and which is part of the GNU Coreutils in the file 'lib/u64.h'
 *  and can be downloaded e.g. from
 *
 *  http://www.gnu.org/software/coreutils/
 *
 *  What of the rest may be new is
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

#define NEED_U64_SET
#define NEED_U64_PLUS
#define NEED_U64_SIZET_PLUS
#define NEED_U64_LT
#define NEED_U64_SHR
#define NEED_U64_LOW

#include <ext/sha256.h>

/* Circular right rotation of 32-bit value 'val' left by 'bits' bits
   (assumes that 'bits' is always within range from 0 to 32) */

#define ROTR(bits, val)  (((val) >> (bits)) | SHA_T32((val) << (32 - (bits))))


/* Right shift of 32-bit value 'val' left by 'bits' bits
   (assumes that 'bits' is always within range from 0 to 32) */

#define SHR(bits, val)  ((val) >> (bits))


/* Initial hash values (see p. 15 of FIPS 180-3) */

static const sha_u32 _H[8] = {0x6a09e667,
                             0xbb67ae85,
                             0x3c6ef372,
                             0xa54ff53a,
                             0x510e527f,
                             0x9b05688c,
                             0x1f83d9ab,
                             0x5be0cd19};

/* Constants required for hash calculation (see p. 11 of FIPS 180-3) */

static const sha_u32 K[64] = {0x428a2f98,
                              0x71374491,
                              0xb5c0fbcf,
                              0xe9b5dba5,
                              0x3956c25b,
                              0x59f111f1,
                              0x923f82a4,
                              0xab1c5ed5,
                              0xd807aa98,
                              0x12835b01,
                              0x243185be,
                              0x550c7dc3,
                              0x72be5d74,
                              0x80deb1fe,
                              0x9bdc06a7,
                              0xc19bf174,
                              0xe49b69c1,
                              0xefbe4786,
                              0x0fc19dc6,
                              0x240ca1cc,
                              0x2de92c6f,
                              0x4a7484aa,
                              0x5cb0a9dc,
                              0x76f988da,
                              0x983e5152,
                              0xa831c66d,
                              0xb00327c8,
                              0xbf597fc7,
                              0xc6e00bf3,
                              0xd5a79147,
                              0x06ca6351,
                              0x14292967,
                              0x27b70a85,
                              0x2e1b2138,
                              0x4d2c6dfc,
                              0x53380d13,
                              0x650a7354,
                              0x766a0abb,
                              0x81c2c92e,
                              0x92722c85,
                              0xa2bfe8a1,
                              0xa81a664b,
                              0xc24b8b70,
                              0xc76c51a3,
                              0xd192e819,
                              0xd6990624,
                              0xf40e3585,
                              0x106aa070,
                              0x19a4c116,
                              0x1e376c08,
                              0x2748774c,
                              0x34b0bcb5,
                              0x391c0cb3,
                              0x4ed8aa4a,
                              0x5b9cca4f,
                              0x682e6ff3,
                              0x748f82ee,
                              0x78a5636f,
                              0x84c87814,
                              0x8cc70208,
                              0x90befffa,
                              0xa4506ceb,
                              0xbef9a3f7,
                              0xc67178f2};


/* Local functions */

static void sha256_process_block(SHA256_Context * context);
static void sha256_evaluate(SHA256_Context * context);


/*----------------------------------------------------------------*
 * Sets up the context structure (or resets it)
 *----------------------------------------------------------------*/

int
sha256_initialize(SHA256_Context * context)
{
    if (! context)
        return SHA_DIGEST_INVALID_ARG;

    memcpy(context->H, _H, sizeof _H);
    context->count         = sha_u64_set(0, 0);
    context->off_count     = 0;
    context->index         = 0;
    context->is_calculated = 0;
    context->error         = SHA_DIGEST_OK;

    return SHA_DIGEST_OK;
}


/*----------------------------------------------------------------*
 * Adds byte-oriented data for the calculation of the hash
 *----------------------------------------------------------------*/

int
sha256_add_bytes(SHA256_Context * context,
                 const void     * data,
                 size_t           num_bytes)
{
    if (! context || ! data)
        return SHA_DIGEST_INVALID_ARG;

    if (context->error)
        return context->error;

    if (context->is_calculated)
        return context->error = SHA_DIGEST_NO_MORE_DATA;

    /* If necessary use bit-oriented routine */

    if (context->off_count != 0)
        return sha256_add_bits(context, data, 8 * num_bytes);

    /* Split up the input into 512 bit sized chunks from which the hash
       value gets calculated */

    while (num_bytes) {
        size_t len = num_bytes >= 64 ? 64 : num_bytes;

        if (context->index + len > 64)
            len = 64 - context->index;

        memcpy(context->buf + context->index, data, len);

        /* Increment bit count, abort on input of 2^64 or more bits */

        context->count = sha_u64_plus(context->count,
                                      sha_u64_set(0, 8 * len));
        if (sha_u64_lt(context->count, sha_u64_set(0, 8 * len)))
            return context->error = SHA_DIGEST_INPUT_TOO_LONG;

        data       = (unsigned char *) data + len;
        num_bytes -= len;

        if ((context->index += len) == 64)
            sha256_process_block(context);
    }

    return SHA_DIGEST_OK;
}


/*----------------------------------------------------------------*
 * Adds bit-oriented data for the calculation of the hash
 *----------------------------------------------------------------*/

int
sha256_add_bits(SHA256_Context * context,
                const void     * data,
                size_t           num_bits)
{
    const unsigned char *d = data;
    unsigned char shift;

    if (! context || ! data)
        return SHA_DIGEST_INVALID_ARG;

    if (context->error)
        return context->error;

    if (context->is_calculated)
        return context->error = SHA_DIGEST_NO_MORE_DATA;

    /* Always try to use the byte-oriented routine as far as possible */

    if (context->off_count == 0 && num_bits > 7)
    {
        int ret = sha256_add_bytes(context, data, num_bits / 8);

        if (ret != SHA_DIGEST_OK|| (num_bits &= 0x07) == 0)
            return ret;

        d += num_bits / 8;
    }

    if (num_bits == 0)
        return SHA_DIGEST_OK;

    /* Increment bit count, abort on input of 2^64 or more bits (note:
       sha_u64_sizet_add() returns a 0 value on overflow!) */

    context->count = sha_u64_sizet_plus(context->count, num_bits);
    if (sha_u64_eq(context->count, sha_u64_set(0, 0)))
        return context->error = SHA_DIGEST_INPUT_TOO_LONG;

    /* Deal with all full (8-bit) bytes of input */

    shift = 8 - context->off_count;
    while (num_bits > 7) {
        context->buf[context->index++] |= SHA_T8(*d) >> context->off_count;
        context->buf[context->index  ]  = (unsigned char)(*d++ << shift);
        num_bits -= 8;

        if (context->index == 64) {
            sha256_process_block(context);
            context->buf[0] = context->buf[64];
        }
    }

    /* Now we're left with no more that 7 bits, they may or may not fit into
       the current byte of the context's buffer */

    if (num_bits > 0) {
        if (num_bits <= shift) {
            if (context->off_count == 0)
                context->buf[context->index] = 0;

            context->buf[context->index] |=
                          (unsigned char)(SHA_T8(*d) >> (8 - num_bits)) << (shift - num_bits);

            if ((context->off_count += (unsigned char)num_bits) == 8) {
                context->off_count = 0;
                if (++context->index == 64)
                    sha256_process_block(context);
            }
        } else {
            context->buf[context->index++] |= SHA_T8(*d) >> context->off_count;
            context->buf[context->index  ]  = (unsigned char)(*d << shift);

            context->off_count = (context->off_count + num_bits) % 8;

            if (context->index == 64) {
                sha256_process_block(context);
                context->buf[0] = context->buf[64];
            }
        }
    }

    return SHA_DIGEST_OK;
}


/*----------------------------------------------------------------*
 * If the digest hadn't been calculated before finalize the
 * calculation, then copy the result iver to the use supplied
 * buffer.
 *----------------------------------------------------------------*/

int
sha256_calculate(SHA256_Context  * context,
                 unsigned char   digest[SHA256_HASH_SIZE])
{
    size_t i, j;

    if (! context || ! digest)
        return SHA_DIGEST_INVALID_ARG;

    if (context->error)
        return context->error;

    if (! context->is_calculated)
        sha256_evaluate(context);

    for (i = j = 0; j < SHA256_HASH_SIZE; i++) {
        digest[j++] = (unsigned char)(context->H[i] >> 24);
        digest[j++] = (unsigned char)(context->H[i] >> 16);
        digest[j++] = (unsigned char)(context->H[i] >>  8);
        digest[j++] = (unsigned char)context->H[i];
    }

    return SHA_DIGEST_OK;
}


/*----------------------------------------------------------------*
 * Central routine for calculating the hash value. See the FIPS
 * 180-3 standard p. 21f for a detailed explanation.
 *----------------------------------------------------------------*/

#define Ch    ((E & F) ^ (SHA_T32(~E) & G))
#define Maj   ((B & C) ^ (B & D) ^ (C & D))
#define Sig0  (ROTR(2, B) ^ ROTR(13, B) ^ ROTR(22, B))
#define Sig1  (ROTR(6, E) ^ ROTR(11, E) ^ ROTR(25, E))

#define sig0(x)  (ROTR( 7, x) ^ ROTR(18, x) ^ SHR( 3, x))
#define sig1(x)  (ROTR(17, x) ^ ROTR(19, x) ^ SHR(10, x))

static void
sha256_process_block(SHA256_Context * context)
{
    size_t         t;
    sha_u32        W[ 64 ];
    unsigned char *buf = context->buf;
    sha_u32        tmp,
                   A = context->H[ 0 ],
                   B = context->H[ 1 ],
                   C = context->H[ 2 ],
                   D = context->H[ 3 ],
                   E = context->H[ 4 ],
                   F = context->H[ 5 ],
                   G = context->H[ 6 ],
                   H = context->H[ 7 ];

    for (t = 0; t < 16; t++) {
        W[t]  = SHA_T8L(*buf++) << 24;
        W[t] |= SHA_T8L(*buf++) << 16;
        W[t] |= SHA_T8L(*buf++) <<  8;
        W[t] |= SHA_T8L(*buf++);

        tmp = SHA_T32(H + Sig1 + Ch + K[t] + W[t]);
        H = G;
        G = F;
        F = E;
        E = SHA_T32(D + tmp);
        D = C;
        C = B;
        B = A;
        A = SHA_T32(tmp + Sig0 + Maj);
    }

    for ( ; t < 64; t++) {
        W[t] = SHA_T32(  sig1(W[t -  2]) + W[t -  7]
                       + sig0(W[t - 15]) + W[t - 16]);

        tmp = SHA_T32(H + Sig1 + Ch + K[t] + W[t]);
        H = G;
        G = F;
        F = E;
        E = SHA_T32(D + tmp);
        D = C;
        C = B;
        B = A;
        A = SHA_T32(tmp + Sig0 + Maj);
    }

    context->H[0] = SHA_T32(context->H[0] + A);
    context->H[1] = SHA_T32(context->H[1] + B);
    context->H[2] = SHA_T32(context->H[2] + C);
    context->H[3] = SHA_T32(context->H[3] + D);
    context->H[4] = SHA_T32(context->H[4] + E);
    context->H[5] = SHA_T32(context->H[5] + F);
    context->H[6] = SHA_T32(context->H[6] + G);
    context->H[7] = SHA_T32(context->H[7] + H);

    context->index = 0;
}


/*----------------------------------------------------------------* 
 * To be called when all data have been entered, applies padding
 * and does the final round of the calculation.
*----------------------------------------------------------------*/

static void
sha256_evaluate(SHA256_Context * context)
{
    int     i;
    sha_u64 count;

    /* If the block is too short for padding (at least one bit plus the
     * bit count as a 64-bit number) padd to the end of the block with 0
     * and then start a new block that contains just 0 and the bit count. */

    if (context->off_count == 0)
        context->buf[context->index++] = 0x80;
    else
        context->buf[context->index++] |= 0x80 >> context->off_count;

    if (context->index > 56) {
        memset(context->buf + context->index, 0, 64 - context->index);
        sha256_process_block(context);
        memset(context->buf, 0, 56);
    } else {
        memset(context->buf + context->index, 0, 56 - context->index);
    }

    /* Store bit count at end and do the final round of the calculation */

    for (count = context->count, i = 63;
         i > 55;
          count = sha_u64_shr(count, 8), i--)
        context->buf[i] = (unsigned char)sha_u64_low(count);

    sha256_process_block(context);
    context->is_calculated = 1;

    /* Wipe memory used for storing data supplied by user */

    memset(context->buf, 0, sizeof context->buf);
}


/*
 * Local variables:
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
