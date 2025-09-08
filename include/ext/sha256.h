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

#if ! defined SHA256_HASH_HEADER_
#define SHA256_HASH_HEADER_

#include <ext/sha_types.h>
#include <stdlib.h>
#include <string.h>


#define SHA256_HASH_SIZE        32


#if ! defined SHA_DIGEST_OK
#define SHA_DIGEST_OK               0
#endif
#if ! defined SHA_DIGEST_INVALID
#define SHA_DIGEST_INVALID_ARG      1
#endif
#if ! defined SHA_DIGEST_INPUT_TOO_LONG
#define SHA_DIGEST_INPUT_TOO_LONG   2
#endif
#if ! defined SHA_DIGEST_NO_MORE_DATA
#define SHA_DIGEST_NO_MORE_DATA     3
#endif


typedef struct {
    sha_u32       H[8];
    sha_u64       count;
    unsigned char off_count;
    unsigned char buf[65];
    size_t        index;
    int           is_calculated;
    int           error;
} SHA256_Context;


#define sha256_add_data sha256_add_bytes

int sha256_initialize(SHA256_Context * context);
int sha256_add_bytes(SHA256_Context * context,
                     const void     * data,
                     size_t           num_bytes);
int sha256_add_bits(SHA256_Context * context,
                    const void     * data,
                    size_t           num_bits);
int sha256_calculate(SHA256_Context * context,
                     unsigned char    digest[SHA256_HASH_SIZE]);

#endif /* ! SHA256_HASH_HEADER_ */


/*
 * Local variables:
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
