/* Copyright 2018 Emil Lenngren. Licensed under the BSD 2-clause license. */

#ifndef P256_CORTEX_ECDH_H
#define P256_CORTEX_ECDH_H

#include <stdbool.h>
#include <stdint.h>

/*
    P-256 ECDH implementation
    =========================

    How to use
    ----------
    Each part generates a key pair:
    uint8_t my_public_point[64], my_private_key[32];
    do {
        generate_random_bytes(my_private_key, 32);
    } while (!P256_ecdh_keygen(my_public_point, my_private_key));
    The function generate_random_bytes is a placeholder for calling the system's cryptographically secure random generator.
    With probability around 1/2^32, the loop will need more than one iteration.

    The public points are then exchanged, and the shared secret is computed:
    uint8_t shared_secret[32];
    if (!P256_ecdh_shared_secret(shared_secret, others_public_point, my_private_key)) {
        // The other part sent an invalid public point, so abort (it is important to handle this case)
    } else {
        // The shared_secret is now the same for both parts and may be used for cryptographic purposes
    }
    A safe alternative if one doesn't want to change the following parts of a protocol if the user sends an invalid key is to replace the
    shared secret with the x coordinate of the own public key (same result as if the remote user sent the basepoint as public key) as follows:
    uint8_t shared_secret[32];
    if (!P256_ecdh_shared_secret(shared_secret, others_public_point, my_private_key)) {
        memcpy(shared_secret, my_public_point, 32);
    }
    // The shared_secret is now the same for both parts and may be used for cryptographic purposes

    About endianness
    ----------------
    All parameters (coordinates, scalars/private keys, shared secret) are represented in little endian byte order.
    Other libraries might use big endian convention, so if this should be used against such a library, make sure all 32-byte values exchanged are reversed.

    Security
    --------
    The implementation runs in constant time (unless input values are invalid) and uses a constant memory access pattern,
    regardless of the scalar/private key in order to protect against side channel attacks.

    Stack usage
    -----------
    All functions need 1356 bytes available stack size on Cortex-M0 and 1.5 kB on Cortex-M4.

*/

// Generic point multiplication
// Calculates scalar * point.
// If include_y_in_result == 0, result_point should be an array of size 32 bytes where the resulting x coordinate will be written.
// If include_y_in_result != 0, result_point should be an array of size 64 bytes where the resulting x coordinate concatenated with y will be written.
// Returns true on success and false on invalid input (see the functions below what defines invalid input).
// If false is returned, the result is not written.
bool P256_pointmult(uint8_t* result_point, const uint8_t point[64], const uint8_t scalar[32], bool include_y_in_result);

// ECDH keygen
// Multiplies the scalar private_key with the curve-defined base point.
// The result should be an array of size 64 bytes where the x coordinate concatenated by the y coordinate will be written.
// Returns true on success and false if the private_key lies outside the allowed range [1..n-1], where n is the curve order.
// If false is returned, the result is not written. (At that point this function can be called again with a new randomized private_key.)
bool P256_ecdh_keygen(uint8_t result_my_public_point[64], const uint8_t private_key[32]);

// ECDH shared secret
// Multiplies the scalar private_key with the other's public point.
// The result should be an array of size 32 bytes where the x coordinate of the result will be written (y is discarded).
// Returns true on success and false if any of the following occurs:
//  - the scalar private_key lies outside the allowed range [1..n-1], where n is the curve order
//  - a public point coordinate integer lies outside the allowed range [0..p-1], where p is the prime for the field used by the curve
//  - the public point does not lie on the curve
// If false is returned, the result is not written.
// NOTE: the boolean return value MUST be checked in order to avoid different attacks.
bool P256_ecdh_shared_secret(uint8_t result_point_x[32], const uint8_t others_public_point[64], const uint8_t private_key[32]) __attribute__((warn_unused_result));

#endif
